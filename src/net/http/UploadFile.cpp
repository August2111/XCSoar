/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "UploadFile.hpp"
#include "Request.hxx"
#include "Handler.hxx"
#include "Operation/Operation.hpp"
#include "io/FileOutputStream.hxx"
#include "thread/Mutex.hxx"
#include "thread/Cond.hxx"
#include "util/NumberParser.hpp"
#include "util/ScopeExit.hxx"
#include "util/StaticString.hxx"

#include <cassert>
#include <limits.h>

class UploadFileHandler final : public CurlResponseHandler {
  uint8_t* buffer;
  const size_t max_size;
  
  size_t posted = 0;
  size_t post_count = 0;

  size_t received = 0;

  OperationEnvironment &env;

  Mutex mutex;
  Cond cond;

  std::exception_ptr error;

  bool done = false;

public:
  UploadFileHandler(void* _buffer, size_t _max_size,
    OperationEnvironment& _env)
    :buffer((uint8_t*)_buffer), max_size(_max_size), env(_env) {}

  void Cancel() noexcept {
    const std::lock_guard<Mutex> lock(mutex);
    done = true;
    cond.notify_one();
  }

  void Wait() {
    std::unique_lock<Mutex> lock(mutex);
    cond.wait(lock, [this]{ return done; });

    if (error)
      std::rethrow_exception(error);
  }

  /* virtual methods from class CurlResponseHandler */
  void OnHeaders(unsigned status,
                 std::multimap<std::string, std::string> &&headers) override {
    if (auto i = headers.find("content-length"); i != headers.end())
      env.SetProgressRange(ParseUint64(i->second.c_str()));
  }

  //-----------------------------------
  void OnData(ConstBuffer<void> data) override {
    size_t remaining = max_size - received;
    if (remaining == 0 || done)
      throw Pause{};

    if (data.size > remaining)
      data.size = remaining;

    memcpy(buffer + received, data.data, data.size);
    received += data.size;

    buffer[received] = 0;
    env.SetProgressRange(received);
  }

  //-----------------------------------
  void OnEnd() override {
    const std::lock_guard<Mutex> lock(mutex);
    done = true;
    if (buffer[0] == 0)
      std::strncpy((char*)buffer, "Upload done!", max_size - 1);
    cond.notify_one();
  }

  //-----------------------------------
  void OnError(std::exception_ptr e) noexcept override {
    const std::lock_guard<Mutex> lock(mutex);
    error = std::move(e);
    try {
      if (error) {
        std::rethrow_exception(error);
      }
    }
    catch (const std::exception& e) {
      std::strncpy((char*)buffer, e.what(), max_size);
    }

    done = true;
    cond.notify_one();    
  }

  //-----------------------------------
  void OnUploadProgress(size_t now, size_t total) {
    if (total > 0 && post_count == 0) {
      post_count = total;
      env.SetProgressRange(total);
    }
    if (posted < now) {
      posted = now;
      env.SetProgressPosition(now);
    }
  }
};

//-----------------------------------------------------------------------------
static int 
OnProgress(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
  UploadFileHandler* handler = reinterpret_cast<UploadFileHandler*> (ptr);
  handler->OnUploadProgress(NowUploaded, TotalToUpload);
  return 0;
}

//-----------------------------------------------------------------------------
static struct curl_httppost* formpost = nullptr;
static struct curl_httppost* lastptr = nullptr;

//-----------------------------------------------------------------------------
static void
UploadFile(CurlGlobal &curl, const TCHAR *url,
               const TCHAR* username, const TCHAR*password,
               void* _buffer, size_t max_length,
               OperationEnvironment &env) {
  assert(url != nullptr);

  UploadFileHandler handler(_buffer, max_length, env);
  CurlRequest request(curl, url, handler);
  request.SetOption(CURLOPT_HTTPPOST, formpost);
 
  request.SetProgress((void*)OnProgress, &handler);
  AtScopeExit(&request) { request.StopIndirect(); };

  request.SetFailOnError();

  if (username != nullptr)
    request.SetOption(CURLOPT_USERNAME, username);
  if (password != nullptr)
    request.SetOption(CURLOPT_PASSWORD, password);

  env.SetCancelHandler([&]{
    request.StopIndirect();
    handler.Cancel();
  });

  AtScopeExit(&env) { env.SetCancelHandler({}); };

  request.StartIndirect();
  handler.Wait();
}

//-----------------------------------------------------------------------------
void
Net::UploadFile(CurlGlobal &curl, const TCHAR *url,
                    const TCHAR *username, const TCHAR *password,
                    Path path, char* buffer, size_t buffer_size,
                    OperationEnvironment &env)
{
  assert(url != nullptr);
  assert(path != nullptr);

  ::UploadFile(curl, url, username, password, buffer, buffer_size, env);

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Net::UploadFileJob::UploadFileJob(CurlGlobal& _curl, const TCHAR * _url, Path _path,
  char* _buffer, size_t _buffer_size)
  :curl(_curl), url(_url), path(_path), buffer(_buffer), buffer_size(_buffer_size)
{
  NarrowString<MAX_PATH> filename;
  filename.SetASCII(path.c_str());
  curl_formfree(formpost);
  curl_formadd(&formpost,
    &lastptr,
    CURLFORM_COPYNAME, "file",
    CURLFORM_FILE, filename,
    CURLFORM_CONTENTTYPE, "application/octet-stream",
    CURLFORM_END);
}


//-----------------------------------------------------------------------------
void
Net::UploadFileJob::Run(OperationEnvironment &env)
{
  UploadFile(curl, url, username, password, path, buffer, buffer_size, env);
}

//-----------------------------------------------------------------------------
void
Net::UploadFileJob::AddValue(const char* name, const char* value) {
  curl_formadd(&formpost,
    &lastptr,
    CURLFORM_COPYNAME, name,
    CURLFORM_COPYCONTENTS, value,
    CURLFORM_END);
}

//-----------------------------------------------------------------------------
#ifdef UNICODE
void
Net::UploadFileJob::AddValue(const char* name, const TCHAR* _value) {
  NarrowString<MAX_PATH> value;
  value.SetASCII(_value);
  AddValue(name, value);
}
#endif  // UNICODE

//-----------------------------------------------------------------------------
void
Net::UploadFileJob::AddValue(const char* name, const uint32_t value) {
  AddValue(name, std::to_string(value).c_str());
}




