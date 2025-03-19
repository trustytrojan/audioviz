#include "state_view.hpp"
#include <SFML/Graphics.hpp>

namespace luaviz
{

state_view::state_view(lua_State *const L)
	: sol::state_view{L}
{
	// clang-format off
#ifdef LINUX
	set("LINUX", true);
#elifdef _WIN32
	set("WIN32", true);
#endif

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

} // namespace luaviz
