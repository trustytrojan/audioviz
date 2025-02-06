#include "lua/audioviz_lua_state_view.hpp"
#include <SFML/Graphics.hpp>

audioviz_lua_state_view::audioviz_lua_state_view(lua_State *L)
	: sol::state_view{L}
{
	// clang-format off
	new_usertype<sf::RenderTarget>(
		"", sol::no_constructor,
		"draw", [](sf::RenderTarget &self, const sf::Drawable &drawable)
		{
			self.draw(drawable);
		}
	);

	new_usertype<sf::Sprite>(
		"", sol::no_constructor,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}
