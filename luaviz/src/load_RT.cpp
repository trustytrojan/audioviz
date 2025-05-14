#include "audioviz/RenderTexture.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_RT()
{
	using RT = RenderTexture;
	// clang-format off
	new_usertype<sf::RenderTarget>("sfRenderTarget",
		"draw", sol::overload(
			[](sf::RenderTarget &self, const sf::Drawable &d, const sf::BlendMode &bm) { self.draw(d, bm); },
			[](sf::RenderTarget &self, const sf::Drawable &d, const sf::RenderStates &rs) { self.draw(d, rs); },
			[](sf::RenderTarget &self, const sf::Drawable &d) { self.draw(d); }
		)
	);

	new_usertype<RT>("RenderTexture",
		sol::base_classes, sol::bases<sf::RenderTarget>(),
		"sprite", &RT::sprite,
		"display", &RT::display,
		"clear", sol::overload(
			static_cast<void (RT::*)(sf::Color)>(&RT::clear),
			[](RT &self, const sol::table &table)
			{
				self.clear(table_to_color(table));
			}
		)
	);
	// clang-format on
}

} // namespace luaviz
