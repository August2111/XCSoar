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

#include "WeGlideServer.hpp"
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
#include "Dialogs/NumberEntry.hpp"
#include "util/StringUtil.hpp"
#include "util/ConvertString.hpp"
#include "system/Path.hpp"
#include "io/FileOutputStream.hxx"
#include "io/FileReader.hxx"
#include "Formatter/TimeFormatter.hpp"
#include "json/ParserOutputStream.hxx"

// TODO(August2111): test only:
#include "Widget/Widget.hpp"
#include "Dialogs/Weather/PCMetDialog.hpp"

#include <type_traits>

#include "UIGlobals.hpp"

// Constructor
WeGlideServer::WeGlideServer() : settings(CommonInterface::GetComputerSettings().weglide) {
}

std::vector<const char *> WeGlideServer::LIST_ITEMS = {"user", "aircraft"};

bool
WeGlideServer::DoUploadToWeGlide(StaticString<CURL_MSG_BUF_SIZE> *msg) {
  StaticString<CURL_MSG_BUF_SIZE> message;
//  char curl_msg[CURL_MSG_BUF_SIZE] = "";
//  char curl_msg[0x10] = "";
  auto igc_path = igcfile_location.c_str();
  bool is_local = igc_path[0] != _T('/');
  msg->append(_T("\n"));
#ifdef _WIN32
  // we are in Windows!
  is_local &= igc_path[0] != _T('\\');
  is_local &= !(igc_path[1] == _T(':') && igc_path[2] == _T('\\'));
  // alternativ:
  is_local &= !(igc_path[1] == _T(':') && igc_path[2] == _T('/'));
#endif
  auto file = is_local ? LocalPath(igcfile_location) :
    AllocatedPath(igcfile_location);
  NarrowString<0x200> url(settings.default_url);
  url += "/igcfile";
  try {
    Json::ParserOutputStream parser;
    Net::UploadFileJob job(*Net::curl, url.c_str(), file, &parser);

    char buffer_string[0x10];
    std::snprintf(buffer_string, sizeof(buffer_string), "%u", pilot_id);
    job.AddPostValue("user_id", buffer_string);
    FormatISO8601(buffer_string, pilot_dob);
    job.AddPostValue("date_of_birth", buffer_string);
    std::snprintf(buffer_string, sizeof(buffer_string), "%u", glider_type);
    job.AddPostValue("aircraft_id", buffer_string);

    DialogJobRunner runner(UIGlobals::GetMainWindow(),
      UIGlobals::GetDialogLook(), _("Upload IGC file to WeGlide"), true);

    if (runner.Run(job)) {
      message = string_converter.Convert(url);
      message.append(_T("\n"));

//      boost::json::standalone::value json = parser.Finish();
      //      if (json.kind() != boost::json::kind::null) {
      Json::Reader json_reader(parser.Finish());
        std::vector<const char *> test_lines = {"id", "competition_id",
                                                "scoring_date", "registration"};
        for (auto entry : test_lines) {
          const auto value = json_reader.GetString(entry);

          if (!value.empty()) {
            char line[0x100];
            std::snprintf(line, sizeof(line), "%20s: %-60s", entry,
                          value.c_str());
            message.append(string_converter.Convert(line));
            message.append(_T("\n"));
          }
        }
        msg->append(message);
        return true;
      } else {
        char msg_str[0x200];
        std::snprintf(msg_str, sizeof(msg_str) - 1,
                      "\n\nURL: '%s' response error!", url.c_str());
        msg->append(string_converter.Convert(msg_str));
        return true;  // false;  // Problems with Exit program?
      }
//       } else {
//         msg->append(_("\nError in DoUploadToWeGlide runner job!"));
//         return true; // false;  // Problems with Exit program?
//       }
  }
  catch (const std::exception& e) {
    // Check for errors
    const char *exc_msg = e.what();
    if (std::strcmp(exc_msg,
      "CURL failed: The requested URL returned error: "
      "406 Not Acceptable") == 0) {
      msg->assign(_("http Error 406 Not Acceptable:\n"
        "The IGC file probably already exists on the server!"));
    } else if (std::strcmp(exc_msg,
      "CURL failed: The requested URL returned error: "
      "400 Bad Request") == 0) {
      msg->assign(_("http Error 400 Bad Request:\n"
        "Probably one of the parameter is wrong!"));
    } else if (std::strcmp(exc_msg,
      "CURL failed: "
      "Failed to open/read local data from file/application") == 0) {
      msg->assign(_("Failed to open/read local data from file/application\n"));
      msg->append(file.c_str());
    } else {
      char msg_str[0x200];
      std::snprintf(msg_str, sizeof(msg_str) - 1, "\nURL: '%s' response error!\n%s",
                    url.c_str(), exc_msg);
      msg->append(string_converter.Convert(msg_str));
    }
    LogFormat(_("WeGlide Upload: %s"), msg->c_str());
    return false;
  }
  catch (...) {
    // Catch unkown errors:
    msg->assign(_("Unknown Exception in Curl Request"));
    LogFormat("WeGlide Upload: Unkown Exception in curl!");
    return false;
  }
}

bool
WeGlideServer::GetWeGlideListItem(const uint32_t id, const ListItem type,
  CurlGlobal& _curl, JobRunner& runner)
{
  bool result = false;
  if (id > 0) {  // a valid pilot or aircraft ID
    char buffer[0x4000];  // 16kB
    NarrowString<0x100> uri;
    uri.Format("%s/%s/%d", settings.default_url, LIST_ITEMS[type], id);
    LogFormat("GetWeGlideListItem: %s", uri.c_str());
    Net::DownloadToBufferJob job(_curl, uri, buffer, sizeof(buffer) - 1);
    result = runner.Run(job);
    if (result) {
      buffer[job.GetLength()] = 0;

      try {
        Json::Reader json_reader(buffer);

        // +++ LOG_ONLY:
        AllocatedPath path = LocalPath(_T(""));
        switch (type) {
        case ENUM_LISTITEM_USER:
          path = LocalPath(_T("logs/User.json")); break;
        case ENUM_LISTITEM_AIRCRAFT:
          path = LocalPath(_T("logs/Glider.json")); break;
        default:
          return result;
        }
        FileOutputStream response(path);
        response.Write(buffer, job.GetLength());
        response.Commit();

        if (id == (uint32_t) json_reader.GetInt("id")) {          
          // id is identical with answered id!
          const auto name = json_reader.GetString("name");
          switch (type) {
          case ENUM_LISTITEM_USER:
            pilot_name =
                string_converter.Convert(name);
            break;
          case ENUM_LISTITEM_AIRCRAFT:
            aircraft_type =
                string_converter.Convert(name);
            break;
          default: break;
          }
        } else {
          LogFormat("WeGlide This was the wrong id in %s.", LIST_ITEMS[type]);
        }
      }
      catch (...) {
        LogFormat("WeGlide no entry '%s' found.", LIST_ITEMS[type]);
      }
    }
  }
  return result;
}

bool
WeGlideServer::Upload2WeGlide(Path path,
  StaticString<CURL_MSG_BUF_SIZE> *message) {
  if (path.IsNull()) {
    *message = _T("Error: Path is Null");
    return false;
  }
  igcfile_location = path;

  return DoUploadToWeGlide(message);
}

bool
WeGlideServer::SetWeGlideUploadParam(uint32_t &pid,
  BrokenDate &date_of_birth, uint32_t &type) {
  const ComputerSettings &settings_computer =
      CommonInterface::GetComputerSettings();
  const WeGlideSettings &weglide = settings_computer.weglide;
  pilot_id = (pid == 0) ? weglide.pilot_id : pid;
  glider_type = (type == 0) ? settings_computer.plane.weglide_glider_type : type;
  pilot_dob = !date_of_birth.IsPlausible() ? weglide.pilot_dob : date_of_birth; 

  pid = pilot_id;
  type = glider_type;
  date_of_birth = pilot_dob;

  return ((pilot_id > 0) && (glider_type > 0) && pilot_dob.IsPlausible());
}

bool
WeGlideServer::DownloadGliderList(char *buffer, size_t buffersize) {
  DialogJobRunner runner(UIGlobals::GetMainWindow(),
    UIGlobals::GetDialogLook(), _("Download GliderList from WeGlide"), true);

  try {
    /* use DownloadToBufferJob because after DownloadToFileJob the
     * file is deleted on error! */
    Net::DownloadToBufferJob job(*Net::curl, settings.gliderlist_uri,
      buffer, buffersize);
    const auto filepath = LocalPath(_T("cache/weglide_gliderlist.csv"));
    bool result = runner.Run(job);
    size_t filesize = 0;
    if (result) {
      filesize = job.GetLength();
      FileOutputStream file(filepath);
      file.Write(buffer, filesize);
      file.Commit();

      LogFormat("WeGlide downloaded gliderlist");
    } else {
      FileReader file(filepath);
      filesize = file.Read(buffer, buffersize);
      if (filesize > 0) {
        LogFormat("WeGlide use old gliderlist: %zu", filesize);
        result = true;
      }
    }
    return result;
  }
  catch (...) {

  }

  return false;
}

bool
WeGlideServer::DownloadTask(uint32_t pilot_id) {
  bool result = false;
  if (pilot_id == 0) {
    // maybe select an other id!
    // TODO: Asking to load the Task with decision from which pilot!!!
    std::unique_ptr<Widget> widget = CreatePCMetWidget();

    pilot_id = settings.pilot_id;  // the preset value

    if (!NumberEntryDialog(_("Selection of Pilot"), pilot_id, 5)) // 5 columns!
      return false;

  }

  if (pilot_id > 0) {
    const auto cache_path = MakeLocalPath(_T("task"));
    NarrowString<0x100> uri(settings.default_url);
    uri.AppendFormat("/task/declaration/%d?cup=false&tsk=true", pilot_id);

    try {
      DialogJobRunner runner(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
        _("Download Task from WeGlide"), true);

      // Net::DownloadToFileJob overwrite the old file!
      char buffer[0x1000];  // = 4kB
      Net::DownloadToBufferJob job(*Net::curl, uri.c_str(), buffer, sizeof(buffer));
      result = runner.Run(job);
      if (result) {
        if (StringCompare(buffer, "null") == 0) {  // current task of pilot is empty!
          ShowMessageBox(_("No declared task available for you!"),
              _("Download Task failed"), MB_OK | MB_ICONINFORMATION);
          result = false;
        } else {
          size_t filesize = job.GetLength();
          FileOutputStream file(LocalPath(_T("task/weglide.tsk")));
          file.Write(buffer, filesize);
          file.Commit();
          LogFormat("WeGlide Task downloaded from pilot '%d'!", pilot_id);
          ShowMessageBox(_("Declared task available was downloaded!"),
                         _("Download Task"), MB_OK | MB_ICONINFORMATION);
        }
      }
    }
    catch (std::exception const& e) {
      // prevent an exception in DownloadJob
      LogFormat("Exception: %s", e.what());
    }
  }

  return result;
}
