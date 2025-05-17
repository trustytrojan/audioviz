#include "table.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace bl = boost::log;

int main(const int argc, const char *const *const argv)
{
	bl::core::get()->set_filter(bl::trivial::severity >= bl::trivial::debug);

	if (argc < 2)
	{
		BOOST_LOG_TRIVIAL(info) << "audioviz lua script required\n";
		return EXIT_FAILURE;
	}

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::os);

	luaviz::table luaviz_tb{lua.create_table()};
	lua["luaviz"] = luaviz_tb;

	// Prepare the arg table
	auto lua_arg{lua.create_table()};
	for (int i = 1; i < argc; ++i) {
		lua_arg[i - 1] = argv[i];
	}
	lua["arg"] = lua_arg;

	lua.safe_script_file(argv[1]);
}
