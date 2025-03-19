#include "table.hpp"
#include "state_view.hpp"

extern "C" int luaopen_luaviz(lua_State *L)
{
	return luaviz::table{luaviz::state_view{L}.create_table()}.push();
}
