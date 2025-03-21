#pragma once

#include <SFML/Graphics.hpp>
#include <functional>

namespace audioviz::util
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);

float weighted_max(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func = {},
	const float size_divisor = 3.5f); // generally the lower third of the frequency spectrum is considered bass

} // namespace audioviz::util
