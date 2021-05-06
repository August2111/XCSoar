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

#include "Cloud/IGCFileUpload.hpp"
#include "Cloud/weglide/WeGlideServer.hpp"
#include "LocalPath.hpp"
#include "Interface.hpp"
#include "Computer/Settings.hpp"
#include "LogFile.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "net/http/Init.hpp"
#include "net/http/ToBuffer.hpp"
#include "Dialogs/JobDialog.hpp"
#include "Dialogs/Error.hpp"
#include "UIGlobals.hpp"
#include "util/StringUtil.hpp"
#include "util/StringCompare.hxx"
#include "util/ConvertString.hpp"
#include "util/StaticString.hxx"
#include "system/Path.hpp"

void
IGCFileUpload::PostIGCFile(Path igcfile_path, uint32_t pilot_id,
  BrokenDate dob, uint32_t plane_id) {
  StaticString<CURL_MSG_BUF_SIZE>_msg;
  const TCHAR *upload_title = _T("Upload flight to WeGlide");
  const TCHAR *download_title = _("Download Pilot and Plane "
    "Info from WeGlide");

  WeGlideServer PostFlight;
  if (!PostFlight.SetWeGlideUploadParam(pilot_id, dob, plane_id))
    return;

  LogFormat(_T("WeGlide Debug pid: %d, aid: %d"), pilot_id, plane_id);

  DialogJobRunner runner(UIGlobals::GetMainWindow(),
    UIGlobals::GetDialogLook(), download_title, true);
  bool success = false;
  try {
    // Look up Pilot and Aircraft
    success = PostFlight.GetWeGlideListItem(pilot_id,
      WeGlideServer::ENUM_LISTITEM_USER, *Net::curl, runner);
    if (success)
      success &= PostFlight.GetWeGlideListItem(plane_id,
        WeGlideServer::ENUM_LISTITEM_AIRCRAFT, *Net::curl, runner);

  }
  catch (...) {
    ShowError(std::current_exception(), _T("WeGlide Upload"));
    LogFormat(_T("WeGlide Upload: No network available"));
    return;
  }

  if (success) {
    _msg.Format(_("IGC file: %s\n"
      "Pilot: %s (Pilot ID: %d)\n"
      "Aircraft: %s (Glider ID: %d)"),
      igcfile_path.c_str(),
      PostFlight.GetPilotName(), pilot_id,
      PostFlight.GetGliderType(), plane_id);

    if (ShowMessageBox(_msg, upload_title, MB_YESNO | MB_ICONQUESTION)
      != IDYES) {
      LogFormat(_T("Abort Upload flight %s"), igcfile_path.c_str());
    } else { // ready to upload IGC file
      PostFlight.Upload2WeGlide(igcfile_path, &_msg);
      ShowMessageBox(_msg, upload_title, MB_OK | MB_ICONINFORMATION);
    }
  }
}
