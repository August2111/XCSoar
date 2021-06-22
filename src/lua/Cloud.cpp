#include "Cloud.hpp"
#include "Util.hxx"
#include "Interface.hpp"
#include "Cloud/weglide/WeGlideSettings.hpp"
#include "Cloud/IGCFileUpload.hpp"
#include "system/Path.hpp"
#include "system/ConvertPathName.hpp"

extern "C" {
#include <lauxlib.h>
}

static int
l_cloud_index(lua_State *L) {
#if 0
  const WeGlideSettings& setting =
    CommonInterface::GetComputerSettings().weglide;

  const char* name = lua_tostring(L, 2);
  if (name == nullptr)
    return 0;
  else if (StringIsEqual(name, "pilot_name")) {
    Lua::Push(L, setting.pilot_name);
  } else if (StringIsEqual(name, "upload2weglide")) {
    Lua::Push(L, setting.pilot_name);
  } else if (StringIsEqual(name, "task_download")) {
    Lua::Push(L, setting.pilot_name);
  } else
    return 0;
#endif

  return 1;
}

static int
l_igcfile_upload(lua_State *L) {
  if (lua_gettop(L) == 0 || lua_gettop(L) > 5)
    return luaL_error(L, "Invalid parameters");
  IGCFileUpload IGCupload(IGCFileUpload::NONE);
  if (strcasecmp(luaL_checkstring(L, 1), "weglide") == 0) {
    IGCupload = IGCFileUpload(IGCFileUpload::WEGLIDE);
  } else {
    return luaL_error(L, "Invalid parameter!");
  }
  StaticString<0x200> str;
  str.SetASCII(luaL_checkstring(L, 2));

  // Pilot ID:
  int pilot_id = lua_gettop(L) > 2 ? luaL_checknumber(L, 3) : 0;
  // Pilot Date of Birth:
  BrokenDate pilot_dob = BrokenDate();
  if (lua_gettop(L) > 3) {
    unsigned day = 0, month = 0, year = 0;
    // Date format should be ISO8601 (2000-01-01)
    if (sscanf(luaL_checkstring(L, 4), "%04u-%02u-%02u", &year, &month, &day)
      == 3) {  // sscanf 3 parameter
      pilot_dob = BrokenDate(year, month, day);
    }
  }
  // Glider ID:
  int glider_id = lua_gettop(L) > 4 ? luaL_checknumber(L, 5) : 0;

  IGCupload.PostIGCFile(Path(str), pilot_id, pilot_dob, glider_id);
  return 0;
}


#include "Cloud/weglide/WeGlideServer.hpp"
static int l_task_download(lua_State *L) {
  if (lua_gettop(L) > 1)
    return luaL_error(L, "Invalid parameters");
  int pilot_id = lua_gettop(L) > 0 ? luaL_checknumber(L, 1) : 0;

  WeGlideServer().DownloadTask(pilot_id);
  return 0;
}

static constexpr struct luaL_Reg settings_funcs[] = {
  {"task_download", l_task_download},
  {"igcfile_upload", l_igcfile_upload},
  {nullptr, nullptr}
};

void
Lua::InitCloud(lua_State *L) {
  lua_getglobal(L, "xcsoar");
  lua_newtable(L);
  lua_newtable(L);
  SetField(L, -2, "__index", l_cloud_index);
  lua_setmetatable(L, -2);

  luaL_setfuncs(L, settings_funcs, 0);
  lua_setfield(L, -2, "cloud");
  lua_pop(L, 1);
}
