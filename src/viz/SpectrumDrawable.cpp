#include "viz/SpectrumDrawable.hpp"

namespace viz
{

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

void SpectrumDrawable::set_rect(const sf::IntRect &rect)
{
	// sanity check
	assert(spectrum.size() == bars.size());
	if (this->rect == rect)
		return;
	this->rect = rect;
	update_bars();
}

void SpectrumDrawable::set_bar_width(int width)
{
	if (bar.width == width)
		return;
	bar.width = width;
	update_bars();
}

void SpectrumDrawable::set_bar_spacing(int spacing)
{
	if (bar.spacing == spacing)
		return;
	bar.spacing = spacing;
	update_bars();
}

void SpectrumDrawable::set_backwards(bool b)
{
	if (backwards == b)
		return;
	backwards = b;
	update_bars();
}

void SpectrumDrawable::update_bars()
{
	// sanity check
	assert(spectrum.size() == bars.size());

	const int bar_count = rect.width / (bar.width + bar.spacing);
	spectrum.resize(bar_count);
	bars.resize(bar_count);

	for (int i = 0; i < bar_count; ++i)
	{
		bars[i].setFillColor(color.get((float)i / bar_count));
		bars[i].setWidth(bar.width);

		// clang-format off
		const auto x = backwards
			? rect.left + rect.width - bar.width - i * (bar.width + bar.spacing)
			: rect.left + i * (bar.width + bar.spacing);
		// clang-format on

		bars[i].setPosition({x, rect.top + rect.height});
	}
}

void SpectrumDrawable::do_fft()
{
	// sanity check
	assert(spectrum.size() == bars.size());

	const int bar_count = spectrum.size();

	fs.render(spectrum);

	for (int i = 0; i < bar_count; ++i)
	{
		// calculate new pill height (sometimes spectrum output is negative)
		auto height = multiplier * rect.height * std::max(0.f, spectrum[i]);

		// don't go over the target's height
		height = std::min((float)rect.height, height);

		bars[i].setHeight(height);
	}
}

void SpectrumDrawable::draw(sf::RenderTarget &target, const sf::RenderStates) const
{
	// sanity check
	assert(spectrum.size() == bars.size());

	for (const auto &pill : bars)
		target.draw(pill);
}

} // namespace viz
