#pragma once

#include <cstdint>
#include <tuple>

namespace ColorUtils
{
	std::tuple<uint8_t, uint8_t, uint8_t> hsvToRgb(float h, const float s, const float v);
	std::tuple<int, int, int> interpolate(float t, float h1, float s1, float v1, float h2, float s2, float v2);
};