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
#include "Cloud/weglide/FileToWeGlide.hpp"
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

#define MY_DEBUG

void
IGCFileUpload::PostIGCFile(Path igcfile_path)
{
  const ComputerSettings& settings_computer = CommonInterface::GetComputerSettings();
  const Plane& plane = settings_computer.plane;
  const WeGlideSettings& weglide = settings_computer.weglide;
  StaticString<0x1000>_msg;
  StaticString<0x40> msg_title;
  msg_title = _T("Upload flight to WeGlide");

#if defined(MY_DEBUG)
  // TOD(August2111)  remove!
  LogFormat(_T("WeGlide Upload MY_DEBUG +++"));
  LogFormat(_T("WeGlide Upload for %s"), plane.registration.c_str());
  LogFormat(_T("IGC file %s"), igcfile_path.c_str());

  LogFormat(_T("WeGlide Upload MY_DEBUG ---"));
#endif // MY_DEBUG

  FileToWeGlide PostFlight;
#if WEGLIDE_PILOT_ID_AS_TEXT  // || 1  // TODO(August2111)!!!!
  // StaticString<10>  _pilot_id(_T("511"));
  uint32_t pilot_id = 511;  //  std::to_integer(_pilot_id.c_str());
  if (!PostFlight.SetWeGlideUploadParam(pilot_id,
#else
  if (!PostFlight.SetWeGlideUploadParam(weglide.pilot_id,
#endif
    weglide.pilot_dob,
    plane.weglide_aircraft_type)) return;

#if 1
  DialogJobRunner runner(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
    _("Download from WeGlide"), true);
#else
  QuietOperationEnvironment openv;
#endif
  bool success = false;
  try {
    /** Look up Pilot and Aircraft */

#if WEGLIDE_PILOT_ID_AS_TEXT  // || 1  // TODO(August2111)!!!!
    uint32_t pilot_id = 511;  //  std::to_integer(_pilot_id.c_str());
    success = PostFlight.GetWeGlideListItem(pilot_id,
#else
    success = PostFlight.GetWeGlideListItem(weglide.pilot_id,
#endif
      FileToWeGlide::ENUM_LISTITEM_USER, *Net::curl, runner);
    if (success)
      success &= PostFlight.GetWeGlideListItem(plane.weglide_aircraft_type,
        FileToWeGlide::ENUM_LISTITEM_AIRCRAFT, *Net::curl, runner);

  }
  catch (...) {
    ShowError(std::current_exception(), _T("WeGlide Upload"));
    LogFormat(_T("WeGlide Upload: No network available"));
    return;
  }

  if (success) {
    _msg.Format(_("Pilot: %s (%d)\nAircraft: %s (%d)"),
      PostFlight.pilot_name.c_str(), weglide.pilot_id,
      PostFlight.aircraft_type.c_str(), plane.weglide_aircraft_type);

    if (ShowMessageBox(_msg, msg_title, MB_YESNO | MB_ICONQUESTION) != IDYES) {
      LogFormat(_T("Abort Upload flight %s"), igcfile_path.c_str());
    } else { // ready to upload IGC file
      PostFlight.Upload2WeGlide(igcfile_path, &_msg, sizeof(_msg));
      ShowMessageBox(_msg, msg_title, MB_OK | MB_ICONINFORMATION);
    }
  }
}
