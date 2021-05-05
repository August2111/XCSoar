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
#include "ToStream.hpp"

#include "Operation/Operation.hpp"
#include "OutputStreamHandler.hxx"
#include "Progress.hpp"
#include "Request.hxx"
#include "util/ScopeExit.hxx"
#include "system/ConvertPathName.hpp"

#include <cassert>

void
Net::UploadFile(CurlGlobal &curl, const char *url,
                const char *username, const char *password,
                http_post* formpost,  Json::ParserOutputStream *parser,
                OperationEnvironment &env)
{
  assert(url != nullptr);

  CurlEasy easy;
  easy.SetURL(url);
  easy.SetFailOnError();

  if (username != nullptr)
    easy.SetOption(CURLOPT_USERNAME, username);
  if (password != nullptr)
    easy.SetOption(CURLOPT_PASSWORD, password);

  easy.SetOption(CURLOPT_HTTPPOST, formpost);


  DownloadToStream(curl, std::move(easy), *parser, env);
}

Net::UploadFileJob::UploadFileJob(CurlGlobal &_curl, const char *_url,
    const Path path, Json::ParserOutputStream *_parser)
    : curl(_curl), url(_url), parser(_parser) {

  assert(path != nullptr);
  curl_formfree(formpost);
  curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE,
      (const char *) NarrowPathName(path), CURLFORM_CONTENTTYPE,
      "application/octet-stream", CURLFORM_END);
}

/** Add a POST option to the upload url. */
void 
Net::UploadFileJob::AddPostValue(const char *name, const char *value) noexcept
{
  curl_formadd(&formpost,
    &lastptr,
    CURLFORM_COPYNAME, name,
    CURLFORM_COPYCONTENTS, value,
    CURLFORM_END);
}

/** Start the upload job. */
void Net::UploadFileJob::Run(OperationEnvironment &env) {
  UploadFile(curl, url, username, password,
             formpost, parser, env);
}
