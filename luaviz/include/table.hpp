#pragma once

#include <SFML/Graphics.hpp>
#include <sol/sol.hpp>

namespace luaviz
{

template <typename T>
sf::Vector2<T> table_to_vec2(const sol::table &tb)
{
	if (tb.empty())
		return {};
	return {tb[1].get<T>(), tb[2].get<T>()};
}

sf::IntRect table_to_intrect(const sol::table &tb);
sf::Color table_to_color(const sol::table &tb);

struct table : sol::table
{
	table();
	table(const sol::table &);
	void load_everything();

	void load_FA();
	void load_AA_SA();
	void load_RT();
	void load_Layer();
	void load_particle_systems();
	void load_CS();
	void load_BSD();
	void load_BSS();
	void load_PSD();
	void load_PSS();
	void load_BSC();
	void load_sf_types();
	void load_Sprite();
	void load_SMD();
	void load_media();
	void load_Base();
	void load_fx();
	void load_tb_conv();
};

} // namespace luaviz
