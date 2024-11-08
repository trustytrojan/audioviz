#pragma once

#include "SFML/System/Vector3.hpp"
#include <SFML/Graphics/Color.hpp>

namespace tt
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
} // namespace tt
