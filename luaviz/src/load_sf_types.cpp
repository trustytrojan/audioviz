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

	new_usertype<sf::RenderStates>("sfRenderStates",
		"new", sol::constructors<sf::RenderStates()>(),
		"blendMode", sol::property(&sf::RenderStates::blendMode, &sf::RenderStates::blendMode),
		"transform", sol::property(&sf::RenderStates::transform, &sf::RenderStates::transform)
	);

	new_usertype<sf::Transform>("sfTransform",
		"", sol::no_constructor,
		"rotateDegrees", [](sf::Transform &self, float degrees)
		{
			self.rotate(sf::degrees(degrees));
		}
	);
	// clang-format on
}

} // namespace luaviz
