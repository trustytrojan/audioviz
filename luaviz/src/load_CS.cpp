#include "audioviz/ColorSettings.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_CS()
{
	using CS = ColorSettings;
	// clang-format off
	new_usertype<CS>("ColorSettings",
		"new", sol::constructors<CS>(),
		"set_mode", &CS::set_mode,
		"set_wheel_hsv", &CS::set_wheel_hsv,
		"set_wheel_ranges_start_hsv", &CS::set_wheel_ranges_start_hsv,
		"set_wheel_ranges_end_hsv", &CS::set_wheel_ranges_end_hsv,
		"set_wheel_rate", &CS::set_wheel_rate,
		"increment_wheel_time", &CS::increment_wheel_time
	);
	// clang-format on
}

} // namespace luaviz
