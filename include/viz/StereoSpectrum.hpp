#pragma once

#include "SpectrumDrawable.hpp"

namespace viz {

class StereoSpectrum : public sf::Drawable
{
	SpectrumDrawable _left, _right;

public:
	StereoSpectrum(int sample_size);
	void set_target_rect(const sf::IntRect &rect);
	void do_fft(const float *const stereo_audio);
	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;
	SpectrumDrawable &left() { return _left; }
	SpectrumDrawable &right() { return _right; }
};

} // namespace viz
