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

#ifndef XCSOAR_FILE_TO_WEGLIDE_HPP
#define XCSOAR_FILE_TO_WEGLIDE_HPP

#include "Language/Language.hpp"
#include "LocalPath.hpp"
#include "io/StringConverter.hpp"
#include "net/http/Init.hpp"
#include "net/http/ToBuffer.hpp"
#include "Job/Runner.hpp"
#include "util/StringUtil.hpp"
#include "system/Path.hpp"
#include "util/StaticString.hxx"
#include "time/BrokenDate.hpp"
#include "json/Reader.hpp"
#include "Cloud/weglide/WeGlideSettings.hpp"

#include <vector>

#define CURL_MSG_BUF_SIZE 0x8000  //  32kB

class WeGlideServer {
 public:
  enum ListItem {
    ENUM_LISTITEM_USER = 0,
    ENUM_LISTITEM_AIRCRAFT
  };

 private:
  Path igcfile_location;

  bool DoUploadToWeGlide(StaticString<CURL_MSG_BUF_SIZE> *msg);
  WeGlideSettings settings;
  uint32_t pilot_id;
  BrokenDate pilot_dob;
  uint32_t glider_type;

  StaticString<0x100> pilot_name;
  StaticString<0x100> aircraft_type;

  static std::vector<const char *> LIST_ITEMS;

  StringConverter string_converter;

 public:
  WeGlideServer();

  const TCHAR *GetGliderType() {
    return aircraft_type.c_str();
  }

  const TCHAR *GetPilotName() {
    return pilot_name.c_str();
  }

  bool SetWeGlideUploadParam(uint32_t &pid,  BrokenDate &dob,
    uint32_t &type);
  bool GetWeGlideListItem(const uint32_t id, ListItem type,
    CurlGlobal& curl, JobRunner& runner);
  bool Upload2WeGlide(Path path, StaticString<CURL_MSG_BUF_SIZE> *message);

  bool DownloadGliderList(char* buffer, size_t buffersize);
  bool DownloadTask(uint32_t pilot_id = 0);
};
#endif