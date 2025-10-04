#include <audioviz/ColorSettings.hpp>
#include <audioviz/util.hpp>

namespace audioviz
{

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
