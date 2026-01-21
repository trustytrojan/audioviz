#include <avz/gfx/util.hpp>
#include <cmath>

namespace avz::util
{

sf::Color hsv2rgb(float h, const float s, const float v)
{
	if (s < 0 || s > 1 || v < 0 || v > 1)
		throw std::invalid_argument("s and v should be in the range [0, 1]");

	// Normalize h to the range [0, 1]
	h = std::fmod(h, 1);

	// Ensure h is not negative
	if (h < 0)
		h += 1;

	float r, g, b;

	if (s == 0)
		r = g = b = v;
	else
	{
		float var_h = h * 6;
		if (var_h == 6)
			var_h = 0;

		int var_i = var_h;
		float var_1 = v * (1 - s);
		float var_2 = v * (1 - s * (var_h - var_i));
		float var_3 = v * (1 - s * (1 - (var_h - var_i)));

		switch (var_i)
		{
		case 0:
			r = v;
			g = var_3;
			b = var_1;
			break;
		case 1:
			r = var_2;
			g = v;
			b = var_1;
			break;
		case 2:
			r = var_1;
			g = v;
			b = var_3;
			break;
		case 3:
			r = var_1;
			g = var_2;
			b = v;
			break;
		case 4:
			r = var_3;
			g = var_1;
			b = v;
			break;
		case 5:
			r = v;
			g = var_1;
			b = var_2;
			break;
		default:
			throw std::logic_error("impossible!!!");
		}
	}

	return {r * 255, g * 255, b * 255};
}

sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv)
{
	const auto [h1, s1, v1] = start_hsv;
	const auto [h2, s2, v2] = end_hsv;

	float h = h1 + std::fmod(t, 1.0) * (h2 - h1);
	float s = s1 + std::fmod(t, 1.0) * (s2 - s1);
	float v = v1 + std::fmod(t, 1.0) * (v2 - v1);

	return {h, s, v};
}

sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv)
{
	const auto [h1, s1, v1] = start_hsv;
	const auto [h2, s2, v2] = end_hsv;
	float reversed_t = std::abs(std::sin(t));
	float h = h1 + reversed_t * (h2 - h1);
	float s = s1 + reversed_t * (s2 - s1);
	float v = v1 + reversed_t * (v2 - v1);
	return {h, s, v};
}
} // namespace avz::util
