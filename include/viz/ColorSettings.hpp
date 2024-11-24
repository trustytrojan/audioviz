#pragma once

#include <SFML/Graphics.hpp>
#include "tt/ColorUtils.hpp"

namespace viz
{

struct ColorSettings
{
	enum class Mode
	{
		WHEEL_RANGES,
		WHEEL,
		WHEEL_RANGES_REVERSE,
		SOLID
	};

	Mode mode = Mode::WHEEL_RANGES_REVERSE;
	sf::Color solid{255, 255, 255};

	/**
	 * @param index_ratio the ratio of your loop index (`i`) to the total number of bars to print (`bars.size()`)
	 */
	sf::Color calculate_color(const float index_ratio) const
	{
		switch (mode)
		{
		case Mode::WHEEL:
		{
			const auto [h, s, v] = wheel.hsv;
			return tt::hsv2rgb(index_ratio + h + wheel.time, s, v);
		}

		case Mode::WHEEL_RANGES:
		{
			const auto [h, s, v] = tt::interpolate(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
			return tt::hsv2rgb(h, s, v);
		}

		case Mode::WHEEL_RANGES_REVERSE:
		{
			const auto [h, s, v] =
				tt::interpolate_and_reverse(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
			return tt::hsv2rgb(h, s, v);
		}

		case Mode::SOLID:
			return solid;

		default:
			throw std::logic_error("SpectrumRenderer::color::get: default case hit");
		}
	}

	// wheel stuff
	struct _wheel
	{
		float time = 0, rate = 0;
		sf::Vector3f hsv{0.9, 0.7, 1}, start_hsv{0.9, 0.7, 1}, end_hsv{.5, .2, 1};
		void increment_time() { time += rate; }
	} wheel;
};

} // namespace viz
