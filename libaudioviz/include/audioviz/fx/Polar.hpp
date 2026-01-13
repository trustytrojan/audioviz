#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz::fx::Polar
{

void setParameters(sf::Vector2f size, float base_radius = 0.f, float max_radius = 1.f);
const sf::Shader &getShader();

} // namespace audioviz::fx::Polar
