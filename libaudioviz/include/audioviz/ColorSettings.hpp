#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz
{

class ColorSettings
{
public:
	enum class Mode
	{
		WHEEL,
		WHEEL_RANGES,
		WHEEL_RANGES_REVERSE,
		SOLID
	};

private:
	Mode mode = Mode::WHEEL;
	sf::Color solid{255, 255, 255};
	struct
	{
		float time = 0, rate = 0;
		sf::Vector3f hsv{0.9, 0.7, 1}, start_hsv{0.9, 0.7, 1}, end_hsv{.5, .2, 1};
	} wheel;

public:
	void set_mode(Mode);
	void set_solid_color(sf::Color);
	void set_wheel_hsv(sf::Vector3f);
	void set_wheel_ranges_start_hsv(sf::Vector3f);
	void set_wheel_ranges_end_hsv(sf::Vector3f);
	void set_wheel_rate(float);
	void increment_wheel_time();

	/**
	 * @param index_ratio the ratio of your loop index (`i`) to the total number of bars to print (`bars.size()`)
	 */
	sf::Color calculate_color(float index_ratio) const;
};

} // namespace viz
