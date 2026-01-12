#pragma once

#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>

namespace audioviz
{

template <typename BarType>
class StereoSpectrum : public sf::Drawable
{
	SpectrumDrawable<BarType> _left, _right;
	sf::IntRect rect;

public:
	StereoSpectrum(const ColorSettings &color)
		: _left{color},
		  _right{color}
	{
	}

	StereoSpectrum(const ColorSettings &colorL, const ColorSettings &colorR)
		: _left{colorL},
		  _right{colorR}
	{
	}

	StereoSpectrum(const sf::IntRect &rect, const ColorSettings &color)
		: _left{color},
		  _right{color},
		  rect{rect}
	{
		update_spectrum_rects();
	}

	StereoSpectrum(const sf::IntRect &rect, const ColorSettings &colorL, const ColorSettings &colorR)
		: _left{colorL},
		  _right{colorR},
		  rect{rect}
	{
		update_spectrum_rects();
	}

	void set_left_backwards(const bool b) { _left.set_backwards(b); }
	void set_right_backwards(const bool b) { _right.set_backwards(b); }

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

	int get_bar_count() const
	{
		assert(_left.bar_count() == _right.bar_count());
		return _left.bar_count();
	}

	void update(FrequencyAnalyzer &fa, StereoAnalyzer &sa)
	{
		assert(_left.bar_count() == _right.bar_count());
		_left.update(fa, sa, 0);
		_right.update(fa, sa, 1);
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override
	{
		target.draw(_left, states);
		target.draw(_right, states);
	}

	void update_spectrum_rects()
	{
		assert(_left.get_bar_spacing() == _right.get_bar_spacing());

		const auto half_width = rect.size.x / 2.f;
		const auto half_bar_spacing = _left.get_bar_spacing() / 2.f;

		// clang-format off
		const sf::IntRect
			left_half{
				rect.position,
				{half_width - half_bar_spacing, rect.size.y}},
			right_half{
				{rect.position.x + half_width + half_bar_spacing, rect.position.y},
				{half_width - half_bar_spacing, rect.size.y}};
		// clang-format on

		const auto dist_between_rects = right_half.position.x - (left_half.position.x + left_half.size.x);
		assert(dist_between_rects == _left.get_bar_spacing());

		_left.set_rect(left_half);
		_right.set_rect(right_half);

		assert(_left.bar_count() == _right.bar_count());
	}
};

} // namespace audioviz
