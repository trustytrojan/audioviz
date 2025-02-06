#pragma once

#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>

sf::Color table_to_color(const sol::table &tb);
sf::Vector2u table_to_vec2u(const sol::table &tb);
sf::IntRect table_to_intrect(const sol::table &tb);
