#include "audioviz/Sprite.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_Sprite()
{
	// clang-format off
	new_usertype<Sprite>("Sprite",
		"new", sol::constructors<Sprite(const sf::Texture &)>(),
		"capture_centered_square_view", &Sprite::capture_centered_square_view,
		"scale_to", [](Sprite &self, const sol::table &size)
		{
			self.scale_to(table_to_vec2<float>(size));
		},
		"fill_screen", [](Sprite &self, const sol::table &size)
		{
			self.fill_screen(table_to_vec2<unsigned>(size));
		},
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
