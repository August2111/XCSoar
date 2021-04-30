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

#include "WeGlideConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Language/Language.hpp"
#include "Tracking/SkyLines/Key.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"

#include "Cloud/weglide/WeGlideSettings.hpp"

#include <stdio.h>

// #define HAVE_WEGLIDE_PILOTNAME

enum ControlIndex {
  WeGlideEnabled,
#ifdef HAVE_WEGLIDE_PILOTNAME
  WeGlidePilotName,
#endif  // HAVE_WEGLIDE_PILOTNAME
  WeGlidePilotBirth,
  WeGlidePilotID,
};


class WeGlideConfigPanel final
  : public RowFormWidget, DataFieldListener {
public:
  WeGlideConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

  void SetEnabled(bool enabled);

  /* virtual methods from class Widget */
  void Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept override;
  bool Save(bool &changed) noexcept override;

private:
  /* methods from DataFieldListener */
  void OnModified(DataField &df) noexcept override;
};

void
WeGlideConfigPanel::SetEnabled(bool enabled)
{
#ifdef HAVE_WEGLIDE_PILOTNAME
  SetRowEnabled(WeGlidePilotName, enabled);
#endif  // HAVE_WEGLIDE_PILOTNAME
  SetRowEnabled(WeGlidePilotBirth, enabled);
  SetRowEnabled(WeGlidePilotID, enabled);
}

void
WeGlideConfigPanel::OnModified(DataField &df) noexcept
{
  if (IsDataField(WeGlideEnabled, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    SetEnabled(dfb.GetAsBoolean());
  }
}

void
WeGlideConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc) noexcept
{
  const ComputerSettings& settings_computer = CommonInterface::GetComputerSettings();
  const WeGlideSettings& weglide = settings_computer.weglide;

  RowFormWidget::Prepare(parent, rc);

  AddBoolean(_T("XCSoar WeGlide"),
      _("Participate in the XCSoar WeGlide field test?  This transmits your location, thermal/wave locations and other weather data to our test server."),
      weglide.enabled == TriState::TRUE,
      this);

#if WEGLIDE_PILOT_ID_AS_TEXT
  AddText(_("WeGlide Pilot ID"),
      _("Take this from your WeGlide Profile. Or set to 0 if not used."),
      weglide.pilot_id);
#else
  AddInteger(_("WeGlide Pilot ID"),
    _("Take this from your WeGlide Profile. Or set to 0 if not used."),
    _T("%u"), _T("%u"),
    0, 10000, 1,
    weglide.pilot_id);
#endif
#if 0  // TODO(August2111): not used up to now
  AddPassword(_("Pilot password"),
      _("Password to login WeGlide."),
      weglide.pilot_key);
#endif
  StaticString<20> birthday = ((BrokenDate)weglide.pilot_dob).c_str();
  AddText(_("Pilot date of birth"),
      _("Date of birth to identify your WeGlide Profile."),
      birthday);
#if 0
  AddText(_("Pilot name at WeGlide"),
      _("The name of pilot to identify your WeGlide Profile."),
      weglide.pilot_name);
#endif

  SetEnabled(weglide.enabled == TriState::TRUE);
}

bool
WeGlideConfigPanel::Save(bool &_changed) noexcept
{
  bool changed = false;

  auto& settings = CommonInterface::SetComputerSettings().weglide;

  bool weglide_enabled = settings.enabled == TriState::TRUE;
  if (SaveValue(WeGlideEnabled, ProfileKeys::WeGlideEnabled, weglide_enabled)) {
    settings.enabled = weglide_enabled
      ? TriState::TRUE
      : TriState::FALSE;

    changed = true;
  }

  _changed |= changed;

  return true;
}

std::unique_ptr<Widget>
CreateWeGlideConfigPanel() {
  return std::make_unique<WeGlideConfigPanel>();
}
