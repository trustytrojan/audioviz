#include <audioviz/ColorSettings.hpp>
#include <audioviz/util.hpp>

namespace audioviz
{

void ColorSettings::set_mode(const Mode mode)
{
	this->mode = mode;
}

void ColorSettings::set_solid_color(const sf::Color color)
{
	solid = color;
}

void ColorSettings::set_wheel_hsv(const sf::Vector3f hsv)
{
	wheel.hsv = hsv;
}

void ColorSettings::set_wheel_ranges_start_hsv(const sf::Vector3f hsv)
{
	wheel.start_hsv = hsv;
}

void ColorSettings::set_wheel_ranges_end_hsv(const sf::Vector3f hsv)
{
	wheel.end_hsv = hsv;
}

void ColorSettings::set_wheel_rate(const float rate)
{
	wheel.rate = rate;
}

void ColorSettings::increment_wheel_time()
{
	wheel.time += wheel.rate;
}

sf::Color ColorSettings::calculate_color(const float index_ratio) const
{
	switch (mode)
	{
	case Mode::WHEEL:
	{
		const auto [h, s, v] = wheel.hsv;
		return util::hsv2rgb(index_ratio + h + wheel.time, s, v);
	}

	case Mode::WHEEL_RANGES:
	{
		const auto [h, s, v] = util::interpolate(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
		return util::hsv2rgb(h, s, v);
	}

	case Mode::WHEEL_RANGES_REVERSE:
	{
		const auto [h, s, v] = util::interpolate_and_reverse(index_ratio + wheel.time, wheel.start_hsv, wheel.end_hsv);
		return util::hsv2rgb(h, s, v);
	}

	case Mode::SOLID:
		return solid;

	default:
		throw std::logic_error("SpectrumRenderer::color::get: default case hit");
	}
}

} // namespace audioviz
