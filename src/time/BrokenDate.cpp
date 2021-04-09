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

#include "BrokenDate.hpp"
#include "BrokenDateTime.hpp"
#include "Calendar.hxx"

#include <cassert>
#include <vector>
#include <algorithm>
#include "boost/algorithm/string.hpp"

#ifdef _UNICODE
typedef std::vector<std::wstring> StringVector;
#else
typedef std::vector<std::string> StringVector;
#endif

BrokenDate::BrokenDate(const TCHAR* date_string) {
  StringVector strs;
  boost::split(strs, date_string, boost::is_any_of(L"/-."));
  if (strs.size() == 3) {
    if (strs.at(0).size() >= 4) {
      year = std::stoi(strs.at(0));
      month = std::stoi(strs.at(1));
      day = std::stoi(strs.at(2));
    }
    else {
      year = std::stoi(strs.at(2));
      month = std::stoi(strs.at(1));
      day = std::stoi(strs.at(0));
    }
  }
}

BrokenDate
BrokenDate::TodayUTC()
{
  return BrokenDateTime::NowUTC();
}

void
BrokenDate::IncrementDay()
{
  assert(IsPlausible());

  const unsigned max_day = DaysInMonth(month, year);

  ++day;

  if (day > max_day) {
    /* roll over to next month */
    day = 1;
    ++month;
    if (month > 12) {
      /* roll over to next year */
      month = 1;
      ++year;
    }
  }

  if (day_of_week >= 0) {
    ++day_of_week;
    if (day_of_week >= 7)
      day_of_week = 0;
  }
}

void
BrokenDate::DecrementDay()
{
  assert(IsPlausible());

  --day;

  if (day < 1) {
    --month;

    if (month < 1) {
      --year;
      month = 12;
    }

    day = DaysInMonth(month, year);
  }
}

int
BrokenDate::DaysSince(const BrokenDate &other) const
{
  constexpr int SECONDS_PER_DAY = 24 * 60 * 60;

  constexpr BrokenTime midnight = BrokenTime::Midnight();
  const int64_t a = BrokenDateTime(*this, midnight).ToUnixTimeUTC();
  const int64_t b = BrokenDateTime(other, midnight).ToUnixTimeUTC();
  const int64_t delta = a - b;
  return int(delta / SECONDS_PER_DAY);
}

StaticString<20>
BrokenDate::c_str() {
  StaticString<20> date_string;
  date_string.Format(_T("%04d-%02d-%02d"), year, month, day);
  return date_string;
}

StaticString<20>
BrokenDate::Format(StaticString<20> format) {
  StaticString<20> date_string;
  // TODO(August2111): make a really format dependend output
  if (format == _T("DD.mm.YYYY")) 
    date_string.Format(_T("%02d.%02.%04d"), day, month, year);
  else if (format == _T("YYYY-mm-DD")) 
    date_string.Format(_T("%04d-%02-%02d"), year, month, day);
  else
    date_string.Format(_T("not defined format: %s"), format);  // TODO(August2111): 
                // Format()- error without arglist
  return date_string;
}
