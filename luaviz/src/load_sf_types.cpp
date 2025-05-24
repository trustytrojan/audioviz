#include <SFML/Graphics.hpp>
#include <audioviz/util.hpp>
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

	new_enum("sfBlendModes",
		"Add", sf::BlendAdd,
		"Alpha", sf::BlendAlpha,
		"Multiply", sf::BlendMultiply,
		"Min", sf::BlendMin,
		"Max", sf::BlendMax,
		"None", sf::BlendNone
	);

	new_enum("sfBlendModeFactor",
		"Zero", sf::BlendMode::Factor::Zero,
		"One", sf::BlendMode::Factor::One,
		"SrcColor", sf::BlendMode::Factor::SrcColor,
		"OneMinusSrcColor", sf::BlendMode::Factor::OneMinusSrcColor,
		"DstColor", sf::BlendMode::Factor::DstColor,
		"OneMinusDstColor", sf::BlendMode::Factor::OneMinusDstColor,
		"SrcAlpha", sf::BlendMode::Factor::SrcAlpha,
		"OneMinusSrcAlpha", sf::BlendMode::Factor::OneMinusSrcAlpha,
		"DstAlpha", sf::BlendMode::Factor::DstAlpha,
		"OneMinusDstAlpha", sf::BlendMode::Factor::OneMinusDstAlpha
	);

	new_enum("sfBlendModeEquation",
		"Add", sf::BlendMode::Equation::Add,
		"Subtract", sf::BlendMode::Equation::Subtract,
		"ReverseSubtract", sf::BlendMode::Equation::ReverseSubtract,
		"Min", sf::BlendMode::Equation::Min,
		"Max", sf::BlendMode::Equation::Max
	);

	new_usertype<sf::BlendMode>("sfBlendMode",
		"new", sol::constructors<sf::BlendMode()>(),
		"colorSrcFactor", &sf::BlendMode::colorSrcFactor,
		"colorDstFactor", &sf::BlendMode::colorDstFactor,
		"colorEquation", &sf::BlendMode::colorEquation,
		"alphaSrcFactor", &sf::BlendMode::alphaSrcFactor,
		"alphaDstFactor", &sf::BlendMode::alphaDstFactor,
		"alphaEquation", &sf::BlendMode::alphaEquation
	);

	set("GreatAmazingBlendMode", &audioviz::util::GreatAmazingBlendMode);

	new_usertype<sf::RenderStates>("sfRenderStates",
		"new", sol::constructors<sf::RenderStates()>(),
		"blendMode", &sf::RenderStates::blendMode,
		"transform", &sf::RenderStates::transform
	);

	new_usertype<sf::Transform>("sfTransform",
		"", sol::no_constructor,
		"rotateDegrees", sol::overload(
			[](sf::Transform &self, float degrees) { self.rotate(sf::degrees(degrees)); },
			[](sf::Transform &self, float degrees, const sol::table &center) { self.rotate(sf::degrees(degrees), table_to_vec2<float>(center)); }
		)
	);

	new_usertype<sf::Texture>("sfTexture",
		"", sol::no_constructor
	);

	new_enum("sfColors",
		"Black", sf::Color::Black,
		"White", sf::Color::White,
		"Red", sf::Color::Red,
		"Green", sf::Color::Green,
		"Blue", sf::Color::Blue,
		"Yellow", sf::Color::Yellow,
		"Magenta", sf::Color::Magenta,
		"Cyan", sf::Color::Cyan,
		"Transparent", sf::Color::Transparent
	);
	// clang-format on
}

} // namespace luaviz
