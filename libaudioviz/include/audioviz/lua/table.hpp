#pragma once

#include <sol/sol.hpp>

namespace audioviz::lua
{

struct table : sol::table
{
	table(const sol::table &);
};

} // namespace audioviz::lua
