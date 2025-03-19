#include "audioviz/RenderTexture.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_RT()
{
	using RT = RenderTexture;
	// clang-format off
	new_usertype<RT>("RenderTexture",
		"new", sol::constructors<RT(sf::Vector2u, int)>(),
		"display", &RT::display,
		"sprite", &RT::sprite,
		"clear", [](RT &self, const sol::table &table)
		{
			self.clear(table_to_color(table));
		},
		sol::base_classes, sol::bases<sf::RenderTarget>()
	);
	// clang-format on
}

} // namespace luaviz
