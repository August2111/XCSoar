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

#include "json/Reader.hpp"

#include <inttypes.h>
// delete(August2111) #include <stdint.h>
// ifdef Bool
//  undef Bool // this predefined type is used in json.hpp again 
// endif
#include <boost/json.hpp>
#ifdef BOOST_JSON_STANDALONE
// #include <boost/json/src.hpp>
#endif
#include "json/Lookup.hxx"

#include <iostream>  // TODO(August2111) only test

namespace json = boost::json;

static void pretty_print(std::ostream &os, json::value const &jv,
                  std::string *indent = nullptr) {
  std::string indent_;
  if (!indent)
    indent = &indent_;
  switch (jv.kind()) {
  case json::kind::object: {
    os << "{\n";
    indent->append(4, ' ');
    auto const &obj = jv.get_object();
    if (!obj.empty()) {
      auto it = obj.begin();
      for (;;) {
        os << *indent << json::serialize(it->key()) << " : ";
        pretty_print(os, it->value(), indent);
        if (++it == obj.end())
          break;
        os << ",\n";
      }
    }
    os << "\n";
    indent->resize(indent->size() - 4);
    os << *indent << "}";
    break;
  }

  case json::kind::array: {
    os << "[\n";
    indent->append(4, ' ');
    auto const &arr = jv.get_array();
    if (!arr.empty()) {
      auto it = arr.begin();
      for (;;) {
        os << *indent;
        pretty_print(os, *it, indent);
        if (++it == arr.end())
          break;
        os << ",\n";
      }
    }
    os << "\n";
    indent->resize(indent->size() - 4);
    os << *indent << "]";
    break;
  }

  case json::kind::string: {
    os << json::serialize(jv.get_string());
    break;
  }

  case json::kind::uint64:
    os << jv.get_uint64();
    break;

  case json::kind::int64:
    os << jv.get_int64();
    break;

  case json::kind::double_:
    os << jv.get_double();
    break;

  case json::kind::bool_:
    if (jv.get_bool())
      os << "true";
    else
      os << "false";
    break;

  case json::kind::null:
    os << "null";
    break;
  }

  if (indent->empty())
    os << "\n";
}
namespace Json {

/**
 * Get a standard string from json value if the type of this value is string,
 * double, uint64 or int64 - otherwise the string is empty!
 */
[[gnu::pure]] 
static std::string 
GetValueAsString(const boost::json::value &value) {
  std::string str;
  char buffer_string[0x10];

  switch (value.kind()) {
  case boost::json::kind::string:
    str = value.get_string();
    break;
  case boost::json::kind::uint64: {
    std::snprintf(buffer_string, sizeof(buffer_string), "%" PRIu64,
                  value.get_uint64());
    str = buffer_string;
  } break;
  case boost::json::kind::int64: {
    std::snprintf(buffer_string, sizeof(buffer_string), "%" PRId64,
                  value.get_int64());
    str = buffer_string;
  } break;
  case boost::json::kind::double_: {
    std::snprintf(buffer_string, sizeof(buffer_string), "%f",
                  value.get_double());
    str = buffer_string;
  } break;
  case boost::json::kind::bool_:
    str = value.get_bool() ? "true" : "false";
    break;
  case boost::json::kind::object:
  case boost::json::kind::array:
  default:
    // in all this cases return an empty string
    break;
  }
  return str;
}

class JsonReader {
  boost::json::value json;

public:
 explicit JsonReader(const boost::json::standalone::value &_json)
{
    json = _json;
    if (json.is_array())
      json = json.get_array().at(0);
  }
  explicit JsonReader(const char *json_string) {
    json = boost::json::parse(json_string);
    if (json.is_array())
      json = json.get_array().at(0);
  }
  explicit JsonReader(Json::ParserOutputStream *parser) {
    json = parser->Finish();
    pretty_print(std::cout, json);
    std::string  str = json.emplace_string().c_str();
    std::cout << str << std::endl;
    if (json.is_array())
      json = json.get_array().at(0);
    str = json.emplace_string().c_str();
    std::cout << str << std::endl;
  }
  std::string GetString(const char *entry) {
    const boost::json::value *value = Json::Lookup(json, entry);
    return Json::GetValueAsString(*value);
  }
  int64_t GetInt(const char *entry) {
    const boost::json::value *value = Json::Lookup(json, entry);
    return value->get_int64();
  }
  const auto *Lookup(const char *entry) { return Json::Lookup(json, entry); }
};

Reader::Reader(const boost::json::standalone::value &_json) {
  json = new JsonReader(_json);
}
Reader::Reader(const char *json_string) { json = new JsonReader(json_string); }

std::string Reader::GetString(const char *entry) {
  return json->GetString(entry);
}
int64_t Reader::GetInt(const char *entry) { return json->GetInt(entry); }

} // namespace Json
