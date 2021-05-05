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

#ifndef NET_HTTP_UPLOADFILE_HPP
#define NET_HTTP_UPLOADFILE_HPP

#include "Job/Job.hpp"
#include "system/Path.hpp"
#include "json/ParserOutputStream.hxx"

#include <array>
#include <cstddef> // for std::byte

class OperationEnvironment;
class CurlGlobal;
typedef struct curl_httppost http_post;

namespace Net {


/**
 * Upload a file to a specified URL in a job.
 * @param   - curl - the curl component
 * @param   - url - the url where the file should be uploaded
 * @param   - path - the path to the upload file
 * @param   - msg_buffer - a char string buffer to receive the message(s)
 *                  from the post convesation, the length should be at least
 *                  the length of the answer, otherwize the end of the message
 *                  will be skipped (maybe very bad to interpret a json text)
 * @param   - msg_buffer_size - the size of the msg_buffer - this is the
 *                  max posible size of characters to be receivable
 *
 */
 void UploadFile(CurlGlobal &curl, const char *url, const char *username,
                const char *password, http_post *formpost,
                Json::ParserOutputStream *parser, OperationEnvironment &env);

static inline void 
UploadFile(CurlGlobal &curl, const char *url,
           http_post *formpost, Json::ParserOutputStream *parser,
           OperationEnvironment &env) {
  UploadFile(curl, url, nullptr, nullptr, formpost, parser, env);
}


class UploadFileJob : public Job {
  CurlGlobal &curl;
  const char *url;
  const char *username = nullptr, *password = nullptr;
  http_post *formpost = nullptr;
  http_post *lastptr = nullptr;
  Json::ParserOutputStream *parser;

public:
  UploadFileJob(CurlGlobal &curl, const char *url, const Path path,
                Json::ParserOutputStream *parser);

  /**
   * Set the upload file job to the autentification mode.
   * @param   - username - username for authentification, if it a nullpointer,
   *                  no authentification is used
   * @param   - password - the password for the specified user
  */
  void SetBasicAuth(const char *_username, const char *_password) noexcept {
    username = _username;
    password = _password;
  }

  /* add a parameter to the POST uri */
  void AddPostValue(const char *name, const char *value) noexcept;

  /* virtual methods from class Job */
  virtual void Run(OperationEnvironment &env);
};

} // namespace Net

#endif  // NET_HTTP_UPLOADFILE_HPP
