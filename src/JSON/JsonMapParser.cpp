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

#include "JsonMapParser.hpp"

#ifdef BOOST_JSON_STANDALONE
# include <boost/json/src.hpp>
#endif

//-----------------------------------------------------------------------------
json_map_t
JsonToMap(boost::json::value json) {
  json_map_t json_map;
  switch (json.kind())
    case boost::json::kind::object: {
    for (auto json_object : json.get_object()) {
      std::string key = json_object.key_c_str();
      auto value = json_object.value();
      switch (value.kind()) {
      case boost::json::kind::string:
        json_map[key] =
          value.get_string().c_str();
        break;
      case boost::json::kind::uint64: {
        uint64_t ui = value.get_uint64();
        json_map[key] = std::to_string(ui);
      } break;
      case boost::json::kind::int64: {
        int64_t i = value.get_int64();
        json_map[key] = std::to_string(i);
      } break;
      case boost::json::kind::double_: {
        double d = value.get_double();
        json_map[key] = std::to_string(d);
      } break;
      case boost::json::kind::bool_:
        json_map[key] =
          value.get_bool() ? "true" : "false";
        break;
      case boost::json::kind::object:
      case boost::json::kind::array:
      default:
        json_map[key] =
          "default: " + std::to_string((uint64_t)value.kind());
        break;
      }
    } break;
    case boost::json::kind::array:
      for (auto json_array : json.get_array()) {
        json_map = JsonToMap(json_array);
        break;
      } break;
    default: break;
  }
  return json_map;
}

//-----------------------------------------------------------------------------
