#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz::fx::Polar
{

void setParameters(
	sf::Vector2f size,
	float base_radius = 0.f,
	float max_radius = 1.f,
	float angle_start = 1.5707963f, // PI/2 - start at top
	float angle_span = 6.2831853f); // 2*PI - full circle
const sf::Shader &getShader();

} // namespace audioviz::fx::Polar
