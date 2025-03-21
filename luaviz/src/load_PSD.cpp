#include "audioviz/SpectrumDrawable.hpp"
#include "audioviz/VerticalPill.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_PSD()
{
	using SD = SpectrumDrawable<VerticalPill>;
	using CS = ColorSettings;
	// clang-format off
	new_usertype<SD>("PillSpectrumDrawable",
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
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
