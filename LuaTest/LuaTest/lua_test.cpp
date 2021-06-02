#include <iostream>
using namespace std;

extern "C" {

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


#pragma comment (lib,"lua54.lib")

int add_in_c(lua_State* L)
{
	int a = lua_tonumber(L, -2);
	int b = lua_tonumber(L, -1);
	lua_pop(L,3);
	int c = a + b;
	lua_pushnumber(L, c);
	return 1;
}

int main()
{

	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadfile(L,"dragon.lua");
	int res = lua_pcall(L, 0, 0, 0);
	if (0 != res) {
		cout << "LUA error in exec : " << lua_tostring(L, -1) << endl;
		exit(-1);
	}
	lua_getglobal(L, "pos_x");
	lua_getglobal(L, "pos_y");
	

	int dragon_x = lua_tonumber(L, -2);
	int dragon_y = lua_tonumber(L, -1);

	lua_pop(L, 2);

	cout << "Position is [" << dragon_x << "," << dragon_y << "]" << endl;

	lua_getglobal(L, "plustwo");
	lua_pushnumber(L, 100);
	lua_pcall(L, 1, 1, 0);
	int result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	cout << "Result is" << result << endl;

	lua_register(L, "call_c_func_add", add_in_c);
	lua_getglobal(L, "add_num");
	lua_pushnumber(L, 100);
	lua_pushnumber(L, 200);
	lua_pcall(L, 2, 1, 0);
	result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	cout << "Result is " << result << endl;
	

	lua_close(L);
}