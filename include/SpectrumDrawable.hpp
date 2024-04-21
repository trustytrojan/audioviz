#pragma once

#include "FrequencySpectrum.hpp"
#include "VerticalPill.hpp"

// would be `sf::Drawable` if its `draw` method wasn't `const`
class SpectrumDrawable
{
	using FS = FrequencySpectrum;

	// spectrum parameters
	struct
	{
		int width = 10, spacing = 5;
	} bar;
	float multiplier = 4;

	// internal data
	FS fs;
	std::vector<float> spectrum;
	std::vector<VerticalPill> pills;

public:
	SpectrumDrawable(int fft_size);

	// setters for SpectrumDrawable
	void set_multiplier(float multiplier);
	void set_bar_width(int width);
	void set_bar_spacing(int spacing);

	// passthrough setters for FrequencySpectrum
	void set_fft_size(int fft_size);
	void set_interp_type(FS::InterpolationType interp_type);
	void set_scale(FS::Scale scale);
	void set_nth_root(int nth_root);
	void set_accum_method(FS::AccumulationMethod method);
	void set_window_func(FS::WindowFunction wf);

	// convenience methods to copy audio first and then draw
	void draw(sf::RenderTarget &target, const float *const audio, int num_channels, int channel, bool interleaved, const sf::RenderStates &states = sf::RenderStates::Default);
	void draw(sf::RenderTarget &target, const float *const audio, const sf::RenderStates &states = sf::RenderStates::Default);

private:
	void draw(sf::RenderTarget &target, const sf::RenderStates &states);
};