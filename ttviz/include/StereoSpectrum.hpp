#pragma once

#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>

class StereoSpectrum : public sf::Drawable
{
	audioviz::SpectrumDrawable _left, _right;
	sf::IntRect rect;

public:
	StereoSpectrum(const sf::IntRect &rect, const audioviz::ColorSettings &color);

	inline void set_left_backwards(const bool b) { _left.set_backwards(b); }
	inline void set_right_backwards(const bool b) { _right.set_backwards(b); }

	inline void set_multiplier(float multiplier)
	{
		_left.set_multiplier(multiplier);
		_right.set_multiplier(multiplier);
	}

	void set_rect(const sf::IntRect &rect);
	void set_bar_width(int width);
	void set_bar_spacing(int spacing);

	inline int get_bar_count() const
	{
		assert(_left.get_bar_count() == _right.get_bar_count());
		return _left.get_bar_count();
	}

	void update(
		audioviz::FrequencyAnalyzer &fa,
		audioviz::StereoAnalyzer &sa,
		audioviz::BinPacker &bp,
		audioviz::Interpolator &ip);

	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;

private:
	void update_spectrum_rects();
};
