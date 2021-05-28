#include <iostream>
using namespace std;

extern "C" {

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


#pragma comment (lib,"lua54.lib")

int main()
{
	const char* lua_pro = "print \"Hello World for LUA\"";
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadbuffer(L,lua_pro,strlen(lua_pro),"line");
	lua_pcall(L, 0, 0, 0);
	lua_close(L);
}