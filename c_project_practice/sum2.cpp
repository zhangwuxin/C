#include <math.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static int ding_sum2(lua_State *L){
    double d1 = luaL_checknumber(L, 1);
    double d2 = luaL_checknumber(L, 2);
    lua_pushnumber(L, d1+d2);
    return 1;
}

static const struct luaL_Reg ding_lib[] = {
    {"ding_sum2" , ding_sum2},
    {NULL, NULL}
};

int luaopen_ding_lib(lua_State *L){
    luaL_newlib(L, ding_lib); // 5.2
    //luaL_register(L, "ding_lib",ding_lib); // lua 5.1
    return 1;
}
