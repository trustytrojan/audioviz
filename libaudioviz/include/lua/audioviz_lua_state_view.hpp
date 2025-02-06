#pragma once

#include <sol/sol.hpp>

struct audioviz_lua_state_view : sol::state_view
{
	audioviz_lua_state_view(lua_State *L);
};
