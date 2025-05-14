#include "table.hpp"

namespace luaviz
{

sf::IntRect table_to_intrect(const sol::table &tb)
{
	const auto pos = tb[1].get<sol::table>(), size = tb[2].get<sol::table>();
	return sf::IntRect{{pos.get_or(1, 0), pos.get_or(2, 0)}, {size.get_or(1, 0), size.get_or(2, 0)}};
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

void table::load_tb_conv()
{
	set_function("table_to_intrect", &table_to_intrect);
	set_function("table_to_color", &table_to_color);
}

} // namespace luaviz
