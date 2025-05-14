#include "audioviz/SpectrumDrawable.hpp"
#include "audioviz/VerticalBar.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_BSD()
{
	using SD = SpectrumDrawable<VerticalBar>;
	using CS = ColorSettings;
	// clang-format off
	new_usertype<SD>("BarSpectrumDrawable",
		sol::base_classes, sol::bases<sf::Drawable>(),
		"new", sol::constructors<SD(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return new SD(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SD::set_multiplier,
		"set_rect", &SD::set_rect,
		"set_bar_width", &SD::set_bar_width,
		"set_bar_spacing", &SD::set_bar_spacing,
		"set_backwards", &SD::set_backwards,
		"configure_analyzer", &SD::configure_analyzer,
		"bar_count", &SD::bar_count,
		"update", &SD::update,
		"update_colors", &SD::update_colors,
		"set_debug_rect", &SD::set_debug_rect
	);
	// clang-format on
}

} // namespace luaviz
