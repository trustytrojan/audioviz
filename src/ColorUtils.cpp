#include <stdexcept>
#include <cmath>
#include "ColorUtils.hpp"

std::tuple<uint8_t, uint8_t, uint8_t> ColorUtils::hsvToRgb(float h, const float s, const float v)
{
	if (s < 0 || s > 1 || v < 0 || v > 1)
		throw std::invalid_argument("s and v should be in the range [0, 1]");

	// Normalize h to the range [0, 1]
	h = std::fmod(h, 1);

	// Ensure h is not negative
	if (h < 0)
		h += 1;

	double r, g, b;

	if (s == 0)
		r = g = b = v;
	else
	{
		double var_h = h * 6;
		if (var_h == 6)
			var_h = 0;

		int var_i = var_h;
		double var_1 = v * (1 - s);
		double var_2 = v * (1 - s * (var_h - var_i));
		double var_3 = v * (1 - s * (1 - (var_h - var_i)));

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

// still haven't integrated this feature yet lmao
// just gonna let it sit on the backburner
std::tuple<int, int, int> ColorUtils::interpolate(float t, float h1, float s1, float v1, float h2, float s2, float v2)
{
	float h = h1 + t * (h2 - h1);
	float s = s1 + t * (s2 - s1);
	float v = v1 + t * (v2 - v1);

	return hsvToRgb(h, s, v);
}