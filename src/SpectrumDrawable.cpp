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

const std::vector<float> &SpectrumDrawable::data() const
{
	return spectrum;
}

void SpectrumDrawable::do_fft(const float *const audio, int num_channels, int channel, bool interleaved)
{
	fs.copy_channel_to_input(audio, num_channels, channel, interleaved);
	do_fft();
}

void SpectrumDrawable::do_fft(const float *const audio)
{
	fs.copy_to_input(audio);
	do_fft();
}

void SpectrumDrawable::set_target_rect(const sf::IntRect &rect, const bool backwards)
{
	// sanity check
	assert(spectrum.size() == pills.size());

	if (target_rect == rect)
		throw std::runtime_error("same rect");

	// fit pills to width of target_rect
	const int pill_count = rect.width / (bar.width + bar.spacing);
	spectrum.resize(pill_count);
	pills.resize(pill_count);

	for (int i = 0; i < pill_count; ++i)
	{
		pills[i].setFillColor(color.get((float)i / pill_count));
		pills[i].setWidth(bar.width);

		const auto x = backwards
						   ? rect.left + rect.width - bar.width - i * (bar.width + bar.spacing)
						   : rect.left + i * (bar.width + bar.spacing);

		pills[i].setPosition({x, rect.top + rect.height});
	}

	target_rect = rect;
}

void SpectrumDrawable::draw(sf::RenderTarget &target, const sf::RenderStates) const
{
	// sanity check
	assert(spectrum.size() == pills.size());

	for (const auto &pill : pills)
		target.draw(pill);
}

void SpectrumDrawable::do_fft()
{
	// sanity check
	assert(spectrum.size() == pills.size());

	const int pill_count = spectrum.size();

	fs.render(spectrum);

	for (int i = 0; i < pill_count; ++i)
	{
		// calculate new pill height (sometimes spectrum output is negative)
		auto height = multiplier * target_rect.height * std::max(0.f, spectrum[i]);

		// don't go over the target's height
		height = std::min((float)target_rect.height, height);

		pills[i].setHeight(height);
	}
}
