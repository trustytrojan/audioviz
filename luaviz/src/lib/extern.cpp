#include "table.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace bl = boost::log;

extern "C" int luaopen_luaviz(lua_State *L)
{
	bl::core::get()->set_filter(bl::trivial::severity >= bl::trivial::debug);
	return luaviz::table{sol::state_view{L}.create_table()}.push();
}
