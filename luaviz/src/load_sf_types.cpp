#include <SFML/Graphics.hpp>
#include <table.hpp>

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

	new_enum("sfBlendMode",
		"Add", sf::BlendAdd,
		"Alpha", sf::BlendAlpha,
		"Multiply", sf::BlendMultiply,
		"Min", sf::BlendMin,
		"Max", sf::BlendMax,
		"None", sf::BlendNone
	);
	// clang-format on
}

} // namespace luaviz
