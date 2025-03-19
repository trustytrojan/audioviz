#include <table.hpp>
#include <SFML/Graphics.hpp>

namespace luaviz
{

void table::load_sf_types()
{
	// clang-format off
	new_usertype<sf::Font>("sfFont",
		"new", sol::constructors<sf::Font(const std::string &)>()
	);

	new_usertype<sf::Text>("sfText",
		"new", sol::constructors<sf::Text(const sf::Font &)>(),
		"setStyle", &sf::Text::setStyle,
		"setCharacterSize", &sf::Text::setCharacterSize,
		"setFillColor", [](sf::Text &self, const sol::table &color)
		{
			self.setFillColor(table_to_color(color));
		}
	);

	new_enum("sfTextStyle",
		"Bold", sf::Text::Bold,
		"Italic", sf::Text::Italic
	);
	// clang-format on
}

} // namespace luaviz
