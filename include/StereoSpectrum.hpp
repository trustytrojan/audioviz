#pragma once

#include "SpectrumDrawable.hpp"

class StereoSpectrum : public sf::Drawable
{
	int sample_size;

	// left and right spectrums
	SpectrumDrawable
		sd_left = sample_size,
		sd_right = sample_size;

public:
	StereoSpectrum(int sample_size);
	void set_target_rect(const sf::IntRect &rect);
	void do_fft(const float *const stereo_audio);
	void draw(sf::RenderTarget& target, sf::RenderStates states = {}) const override;
	SpectrumDrawable &left() { return sd_left; }
	SpectrumDrawable &right() { return sd_right; }
};
