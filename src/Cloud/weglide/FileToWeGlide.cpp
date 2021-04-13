/*
TODO(August2111):
* Abfrage von Datenbreite von TCHAR (Linux, ohne UNICODE,...)
* Job-Managment des Runner-Threads...
* Einstellung von curl mit utf-8 oder widechar möglich?
* Zip-Datei...
*/
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

#include "FileToWeGlide.hpp"
#include "LocalPath.hpp"
#include "Interface.hpp"
#include "Computer/Settings.hpp"
#include "LogFile.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "net/http/Init.hpp"
#include "net/http/ToBuffer.hpp"
#include "net/http/ToFile.hpp"
#include "net/http/UploadFile.hpp"
#include "Dialogs/JobDialog.hpp"
#include "util/StringUtil.hpp"
#include "util/ConvertString.hpp"
#include "system/Path.hpp"
#include <type_traits>

#include "UIGlobals.hpp"

#include <curl/curl.h>


#define CURL_MSG_BUF_SIZE 0x4000  //  16kB  4096

#define STRNCOPY std::strncpy

//-----------------------------------------------------------------------------
// Constructor

FileToWeGlide::FileToWeGlide() {
  // take it from settings:
  default_url = CommonInterface::GetComputerSettings().weglide.default_url;
}

//-----------------------------------------------------------------------------
bool
FileToWeGlide::DoUploadToWeGlide(StaticString<0x1000>* msg, const size_t msg_size) {
  StaticString<CURL_MSG_BUF_SIZE> message;
  char curl_msg[CURL_MSG_BUF_SIZE];
  try {
    bool is_local = igcfile_location[0] != L'/';
#ifdef _WIN32
    is_local &= igcfile_location[0] != L'\\';
    is_local &= !(igcfile_location[1] == L':' && igcfile_location[2] == L'\\');
    // alternativ:
    is_local &= !(igcfile_location[1] == L':' && igcfile_location[2] == L'/');
#endif
    auto file = is_local ? LocalPath(igcfile_location) : AllocatedPath(igcfile_location);
    StaticString<MAX_PATH> url(default_url);
    url += _T("/igcfile");
    strcpy(curl_msg, "Dummy!");  // TODO(August2111): only dummy!
    Net::UploadFileJob job(*Net::curl, url, file, curl_msg, sizeof(curl_msg));
    job.AddValue("user_id", pilot_id);
    job.AddValue("date_of_birth", pilot_dob.c_str());
    job.AddValue("aircraft_id", aircraft_type_id);

    DialogJobRunner runner(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
      _("Upload IGC file to WeGlide"), true);

    runner.Run(job);

    json_map_t json_map = JsonToMap(boost::json::parse(curl_msg));
    message.SetASCII(url);
    message.append(_T("\n"));

    std::vector<const char*> test_lines = { "id", "competition_id", "scoring_date", "registration" };
    for (auto pair : json_map) {
      char line[0x100];
      std::snprintf(line, sizeof(line), "%-20s: %s", pair.first.c_str(), pair.second.c_str());
      LogFormat("WeGlide Upload: %s", line);
      if (find(test_lines.begin(), test_lines.end(), pair.first) != test_lines.end()) {
#ifdef UNICODE
        message.append(ConvertACPToWide(line).c_str());
#else  // UNICODE
        message.append(line);
#endif  // UNICODE
        message.append(_T("\n"));
      }
    }
    msg->SetASCII(message);
    return true;
  }
  catch (...) {
    /* Check for errors */
    msg->SetASCII(curl_msg);
    LogFormat("WeGlide Upload: Exception at parsing curl: %s", curl_msg);
    return false;
  }
}

//-----------------------------------------------------------------------------
static StaticString<0x100>
GetNameFromType(json_map_t* json_map) {
  StaticString<0x100> name;
  auto pair = json_map->find("name"); 
  if (pair != json_map->end()) {
    name.SetASCII(pair->second.c_str());
  } else {
    name = _T("--undefined--");
  }

  return name;
}
//-----------------------------------------------------------------------------
bool
FileToWeGlide::GetWeGlideListItem(const uint32_t id, const ListItem type,
  CurlGlobal& _curl, JobRunner& runner)
{
  bool result = false;
  if (id > 0) { // not a valid pilot or aircraft ID
    std::vector<const TCHAR*> LIST_ITEMS = { _T("user"), _T("aircraft") };
    NarrowString<0x100> uri;
#ifdef _UNICODE  // better: uri could be TCHAR*
    StaticString<0x100> query_URI;
    query_URI.Format(_T("%s/%s/%d"), default_url, LIST_ITEMS[type], id);
    uri.SetASCII(query_URI);  // as NarrowString
#else
    uri.Format(_T("%s/%s/%d"), default_url, LIST_ITEMS[type], id);
#endif

    char buffer[0x1000];
    Net::DownloadToBufferJob job(_curl, uri, buffer, sizeof(buffer) - 1);
    // job.SetBasicAuth(username, password);
    result = runner.Run(job);
    if (result) {
      buffer[job.GetLength()] = 0;

      try {
        boost::json::value json = boost::json::parse(buffer);

        json_map_t json_map = JsonToMap(json);
        switch (type) {
        case ENUM_LISTITEM_USER:
          pilot_name = GetNameFromType(&json_map);
          break;
        case ENUM_LISTITEM_AIRCRAFT:
          aircraft_type = GetNameFromType(&json_map);
          break;
        default: break;
        }
      }
      catch (...) {
        LogFormat("WeGlide no entry '%s' found.", LIST_ITEMS[type]);
      }
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
bool
FileToWeGlide::Upload2WeGlide(Path path, StaticString<0x1000>* message, const size_t msg_size) {
  if (path.IsNull()) {
    *message = _T("Error: Path is Null");
    return false;
  }
  igcfile_location = path.c_str();
  return DoUploadToWeGlide(message, msg_size);
}

//-----------------------------------------------------------------------------
bool
FileToWeGlide::SetWeGlideUploadParam(const uint32_t pid,
    const BrokenDate date_of_birth, const uint32_t type) {

  pilot_dob = BrokenDate(0,0,0);
  if ((pid == 0) || (type == 0) || !date_of_birth.IsPlausible())
    return false;
  pilot_id = pid;
  aircraft_type_id = type;
  pilot_dob = date_of_birth;

  return true;
}

//-----------------------------------------------------------------------------
bool
FileToWeGlide::DownloadTask(void) {
  auto settings = CommonInterface::GetComputerSettings().weglide;

  // TODO(August2111): Asking to load the Task with decision
  // from which pilot!!!
  uint32_t pilot_id = settings.pilot_id;  // the preset value

  const auto cache_path = MakeLocalPath(_T("task"));
  NarrowString<0x100> uri;
  uri.SetASCII(settings.default_url);  // as NarrowString
  uri.AppendFormat("/task/declaration/%d?cup=false&tsk=true", pilot_id);

  const auto filepath = LocalPath(_T("task/weglide.tsk"));
  DialogJobRunner runner(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
    _("Download Task from WeGlide"), true);

  Net::DownloadToFileJob job(*Net::curl, uri, filepath);
  bool result = runner.Run(job);
  if (result) {
    LogFormat("WeGlide Task downloaded from pilot '%d'!", pilot_id);
  }

  return result;
}

//-----------------------------------------------------------------------------
