#include "audioviz/ScopeDrawable.hpp"
#include "audioviz/VerticalBar.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_BSC()
{
	using SD = ScopeDrawable<VerticalBar>;
	using CS = ColorSettings;
	// clang-format off
	new_usertype<SD>("BarScopeDrawable",
		"new", sol::constructors<SD(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return new SD(table_to_intrect(rect), cs);
		}),
		"set_rect", &SD::set_rect,
		"set_shape_width", &SD::set_shape_width,
		"set_shape_spacing", &SD::set_shape_spacing,
		"set_fill_in", &SD::set_fill_in,
		"set_backwards", &SD::set_backwards,
		"set_rotation_angle", &SD::set_rotation_angle,
		"set_center_point", &SD::set_center_point,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
