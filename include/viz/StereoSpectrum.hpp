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
	StereoSpectrum(ColorSettings &color)
		: _left{color},
		  _right{color}
	{
	}

	StereoSpectrum(const sf::IntRect &rect, const ColorSettings &color)
		: _left{rect, color},
		  _right{rect, color}
	{
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

	/**
	 * Resizes `sa` 's spectrum vectors to be the same size as
	 * the number of bars that this `StereoSpectrum` is setup to
	 * render. You MUST call this before analyzing audio, otherwise
	 * you might get an assertion error from `tt::AudioAnalyzer::analyze`.
	 */
	void configure_analyzer(tt::StereoAnalyzer &sa)
	{
		assert(_left.bar_count() == _right.bar_count());
		sa.resize(_left.bar_count());
	}

	/**
	 * @warning It is the CALLER's responsibility to make sure that
	 * `sa`'s spectrum buffers are properly sized for this `StereoSpectrum`!!!!
	 * Otherwise you will get an assertion error!!! Call `before_analyze` to help you with this.
	 */
	void update(const tt::StereoAnalyzer &sa)
	{
		_left.update(sa.left_data());
		_right.update(sa.right_data());
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override
	{
		target.draw(_left, states);
		target.draw(_right, states);
	}

private:
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

} // namespace viz
