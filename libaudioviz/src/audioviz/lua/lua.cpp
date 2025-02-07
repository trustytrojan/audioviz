#include <audioviz/lua/table.hpp>
#include <audioviz/lua/state_view.hpp>

extern "C" int luaopen_audioviz(lua_State *L)
{
	return audioviz::lua::table{audioviz::lua::state_view{L}.create_table()}.push();
}
