#include "SpectrumDrawable.hpp"

SpectrumDrawable::SpectrumDrawable(const int fft_size)
	: fs(fft_size) {}

void SpectrumDrawable::set_multiplier(float multiplier)
{
	this->multiplier = multiplier;
}

void SpectrumDrawable::set_fft_size(int fft_size)
{
	fs.set_fft_size(fft_size);
}

void SpectrumDrawable::set_interp_type(FS::InterpolationType interp_type)
{
	fs.set_interp_type(interp_type);
}

void SpectrumDrawable::set_scale(FS::Scale scale)
{
	fs.set_scale(scale);
}

void SpectrumDrawable::set_nth_root(int nth_root)
{
	fs.set_nth_root(nth_root);
}

void SpectrumDrawable::set_accum_method(FS::AccumulationMethod method)
{
	fs.set_accum_method(method);
}

void SpectrumDrawable::set_window_func(FS::WindowFunction wf)
{
	fs.set_window_func(wf);
}

void SpectrumDrawable::copy_channel_to_input(const float *const audio, int num_channels, int channel, bool interleaved)
{
	fs.copy_channel_to_input(audio, num_channels, channel, interleaved);
}

void SpectrumDrawable::copy_to_input(const float *const audio)
{
	fs.copy_to_input(audio);
}

void SpectrumDrawable::draw(sf::RenderTarget &target, sf::IntRect rect, bool backwards)
{
	// sanity check
	assert(spectrum.size() == pills.size());

	// if not provided (or zero size), use the whole target
	if (rect.width == 0 || rect.height == 0)
		rect = {{}, (sf::Vector2i)target.getSize()};

	const int pill_count = rect.width / (bar.width + bar.spacing);
	spectrum.resize(pill_count);
	pills.resize(pill_count);
	fs.render(spectrum);

	for (int i = 0; i < pill_count; ++i)
	{
		pills[i].setFillColor(color.get((float)i / pill_count));
		pills[i].setWidth(bar.width);

		const auto x = backwards
			? (rect.left + rect.width) - i * (bar.width + bar.spacing)
			: rect.left + i * (bar.width + bar.spacing);

		pills[i].setPosition({x, rect.height});

		// calculate new pill height (sometimes spectrum output is negative)
		auto height = multiplier * rect.height * std::max(0.f, spectrum[i]);

		// don't go over the target's height
		height = std::min((float)rect.height, height);

		pills[i].setHeight(height);
		target.draw(pills[i]);
	}
}