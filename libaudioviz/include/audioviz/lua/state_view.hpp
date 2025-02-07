#pragma once

#include <sol/sol.hpp>

namespace audioviz::lua
{

struct state_view : sol::state_view
{
	state_view(lua_State *);
};

} // namespace audioviz::lua
