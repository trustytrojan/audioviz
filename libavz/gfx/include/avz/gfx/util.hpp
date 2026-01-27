#pragma once

#include <SFML/Graphics.hpp>

namespace avz::util
{

inline constexpr sf::String utf8_to_sf_string(const std::string &text)
{
	return sf::String::fromUtf8(text.begin(), text.end());
}

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);

inline const sf::BlendMode GreatAmazingBlendMode{sf::BlendMode::Factor::OneMinusDstColor, sf::BlendMode::Factor::One};

} // namespace avz::util
