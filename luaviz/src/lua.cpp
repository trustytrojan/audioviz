#include "table.hpp"

extern "C" int luaopen_luaviz(lua_State *L)
{
	return luaviz::table{sol::state_view{L}.create_table()}.push();
}
