#pragma once

#include <sol/sol.hpp>

namespace luaviz
{

struct state_view : sol::state_view
{
	state_view(lua_State *);
};

} // namespace luaviz
