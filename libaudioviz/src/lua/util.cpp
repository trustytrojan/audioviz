#include "lua/util.hpp"

sf::IntRect table_to_intrect(const sol::table &tb)
{
	const auto pos = tb[1].get<sol::table>(), size = tb[2].get<sol::table>();
	return {
		pos.empty() ? sf::Vector2i{} : sf::Vector2i{pos[1].get<int>(), pos[2].get<int>()},
		size.empty() ? sf::Vector2i{} : sf::Vector2i{size[1].get<int>(), size[2].get<int>()},
	};
}

sf::Vector2u table_to_vec2u(const sol::table &tb)
{
	if (tb.empty())
		return {};
	return {tb[1].get<uint>(), tb[2].get<uint>()};
}

sf::Color table_to_color(const sol::table &tb)
{
	auto color = sf::Color::Black;
	if (tb[1].valid())
		color.r = tb[1];
	if (tb[2].valid())
		color.g = tb[2];
	if (tb[3].valid())
		color.b = tb[3];
	if (tb[4].valid())
		color.a = tb[4];
	if (tb["r"].valid())
		color.r = tb["r"];
	if (tb["g"].valid())
		color.r = tb["g"];
	if (tb["b"].valid())
		color.b = tb["b"];
	if (tb["a"].valid())
		color.a = tb["a"];
	return color;
}
