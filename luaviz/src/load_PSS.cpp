#include "audioviz/StereoSpectrum.hpp"
#include "audioviz/VerticalPill.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_PSS()
{
	using SS = StereoSpectrum<VerticalPill>;
	using CS = ColorSettings;
	// clang-format off
	new_usertype<SS>("BarStereoSpectrum",
		"new", sol::constructors<SS(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return new SS(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SS::set_multiplier,
		"set_rect", [](SS &self, const sol::table &rect)
		{
			self.set_rect(table_to_intrect(rect));
		},
		"set_bar_width", &SS::set_bar_width,
		"set_bar_spacing", &SS::set_bar_spacing,
		"set_left_backwards", &SS::set_left_backwards,
		"set_right_backwards", &SS::set_right_backwards,
		"update", &SS::update,
		"get_bar_count", &SS::get_bar_count,
		"configure_analyzer", &SS::configure_analyzer,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
