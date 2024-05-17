#pragma once

#include "SpectrumDrawable.hpp"

namespace viz
{

class StereoSpectrum : public sf::Drawable
{
	SpectrumDrawable _left, _right;
	sf::IntRect rect;

public:
	StereoSpectrum(int sample_size);
	void set_bar_spacing(int spacing);
	void set_rect(const sf::IntRect &rect);
	void do_fft(const float *const stereo_audio);
	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;
	SpectrumDrawable &left() { return _left; }
	SpectrumDrawable &right() { return _right; }

private:
	void update_spectrums();
};

} // namespace viz
