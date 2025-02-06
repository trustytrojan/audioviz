#include "lua/audioviz_lua_table.hpp"
#include "lua/audioviz_lua_state_view.hpp"

extern "C" int luaopen_audioviz(lua_State *L)
{
	return audioviz_lua_table{audioviz_lua_state_view{L}.create_table()}.push();
}
