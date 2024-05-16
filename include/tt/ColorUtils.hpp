#pragma once

#include <SFML/Graphics/Color.hpp>

namespace tt
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Color interpolate(float t, float h1, float s1, float v1, float h2, float s2, float v2);

} // namespace tt
