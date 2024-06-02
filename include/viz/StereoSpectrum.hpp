#pragma once

#include "tt/StereoAnalyzer.hpp"

#include "SpectrumDrawable.hpp"

namespace viz
{

template <typename BarType>
class StereoSpectrum : public sf::Drawable
{
	using SD = SpectrumDrawable<BarType>;

	SD _left, _right;
	sf::IntRect rect;

public:
	StereoSpectrum()
	{
		_left.set_backwards(true);
	}

	void set_bar_width(int width)
	{
		_left.set_bar_width(width);
		_right.set_bar_width(width);
		update_spectrum_rects();
	}

	void set_bar_spacing(int spacing)
	{
		_left.set_bar_spacing(spacing);
		_right.set_bar_spacing(spacing);
		update_spectrum_rects();
	}

	void set_color_mode(SD::ColorMode mode)
	{
		_left.color.set_mode(mode);
		_right.color.set_mode(mode);
	}

	void set_solid_color(sf::Color color)
	{
		_left.color.set_solid_rgb(color);
		_right.color.set_solid_rgb(color);
	}

	void set_color_wheel_rate(float rate)
	{
		_left.color.wheel.set_rate(rate);
		_right.color.wheel.set_rate(rate);
	}

	void set_color_wheel_hsv(sf::Vector3f hsv)
	{
		_left.color.wheel.set_hsv(hsv);
		_right.color.wheel.set_hsv(hsv);
	}

	void set_multiplier(float multiplier)
	{
		_left.set_multiplier(multiplier);
		_right.set_multiplier(multiplier);
	}

	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_spectrum_rects();
	}

	void process(tt::FrequencyAnalyzer &fa, tt::StereoAnalyzer &sa, const float *const audio)
	{
		assert(_left.bar_count() == _right.bar_count());
		sa.resize(_left.bar_count());
		sa.analyze(fa, audio);
		_left.update_bar_heights(sa.left_data());
		_right.update_bar_heights(sa.right_data());
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override
	{
		target.draw(_left, states);
		target.draw(_right, states);
	}

private:
	void update_spectrum_rects()
	{
		assert(_left.bar.get_spacing() == _right.bar.get_spacing());

		const auto half_width = rect.width / 2.f;
		const auto half_bar_spacing = _left.bar.get_spacing() / 2.f;

		const sf::IntRect
			left_half{
				rect.getPosition(),
				{half_width - half_bar_spacing, rect.height}},
			right_half{
				{rect.left + half_width + half_bar_spacing, rect.top},
				{half_width - half_bar_spacing, rect.height}};

		const auto dist_between_rects = right_half.left - (left_half.left + left_half.width);
		assert(dist_between_rects == _left.bar.get_spacing());

		_left.set_rect(left_half);
		_right.set_rect(right_half);

		assert(_left.bar_count() == _right.bar_count());
	}
};

} // namespace viz
