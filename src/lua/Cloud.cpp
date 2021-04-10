#include "Cloud.hpp"   // WeGlide
#include "Util.hxx"
#include "Interface.hpp"
#include "Cloud/IGCFileUpload.hpp"   // WeGlide
#include "system/Path.hpp"

extern "C" {
#include <lauxlib.h>
}


static int
l_cloud_index(lua_State* L)
{
  /**/
//  const ComputerSettings& settings_computer = CommonInterface::GetComputerSettings();
//  const LoggerSettings& logger = settings_computer.logger;

  const char* name = lua_tostring(L, 2);
  if (name == nullptr)
    return 0;
  else if (StringIsEqual(name, "pilot_name")) {
    Lua::Push(L, "Uwe Augustin");  // TODO(August2111) REMOVE!!!
  }
  else
    return 0;

  return 1;
}


static int
l_upload2weglide(lua_State* L)
{
  if (lua_gettop(L) != 1)
    return luaL_error(L, "Invalid parameters");

  //  IGCFileUpload IGCupload(IGCFileUpload::CompPlatform::WeGlide);
  IGCFileUpload IGCupload(IGCFileUpload::WEGLIDE);
  StaticString<0x100> str;
  // ??? str.SetASCII(luaL_checkstring(L, 1));
  str.SetUTF8(luaL_checkstring(L, 1));
  IGCupload.PostIGCFile(Path(str));
  return 0;
}

static constexpr struct luaL_Reg settings_funcs[] = {
  {"upload2weglide", l_upload2weglide},
  {nullptr, nullptr}
};
void
Lua::InitCloud(lua_State* L)
{
  lua_getglobal(L, "xcsoar");
  lua_newtable(L);
  lua_newtable(L);
  SetField(L, -2, "__index", l_cloud_index);
  lua_setmetatable(L, -2);

  luaL_setfuncs(L, settings_funcs, 0);
  lua_setfield(L, -2, "cloud");
  lua_pop(L, 1);
}
