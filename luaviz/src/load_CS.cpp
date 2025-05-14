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
		"set_solid_color", sol::overload(
			&CS::set_solid_color,
			[](CS &self, const sol::table &color)
			{
				self.set_solid_color(table_to_color(color));
			}
		),
		"set_wheel_hsv", &CS::set_wheel_hsv,
		"set_wheel_ranges_start_hsv", &CS::set_wheel_ranges_start_hsv,
		"set_wheel_ranges_end_hsv", &CS::set_wheel_ranges_end_hsv,
		"set_wheel_rate", &CS::set_wheel_rate,
		"increment_wheel_time", &CS::increment_wheel_time
	);

	new_enum("ColorMode",
		"SOLID", CS::Mode::SOLID,
		"WHEEL", CS::Mode::WHEEL,
		"WHEEL_RANGES", CS::Mode::WHEEL_RANGES,
		"WHEEL_RANGES_REVERSE", CS::Mode::WHEEL_RANGES_REVERSE
	);
	// clang-format on
}

} // namespace luaviz
