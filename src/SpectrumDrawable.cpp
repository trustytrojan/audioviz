#include "SpectrumDrawable.hpp"

SpectrumDrawable::SpectrumDrawable(const int fft_size)
	: fs(fft_size) {}

void SpectrumDrawable::draw(sf::RenderTarget &target, const float *const audio, const int num_channels, const int channel, const bool interleaved)
{
	fs.copy_channel_to_input(audio, num_channels, channel, interleaved);
	draw(target);
}

void SpectrumDrawable::draw(sf::RenderTarget &target, const float *const audio)
{
	fs.copy_to_input(audio);
	draw(target);
}

void SpectrumDrawable::draw(sf::RenderTarget &target)
{
	// sanity check
	assert(spectrum.size() == pills.size());

	const int old_pill_count = spectrum.size();
	const auto target_size = sf::Vector2f(target.getSize());
	const int pill_count = target_size.x / (bar.width + bar.spacing);

	if (old_pill_count != pill_count)
	{
		spectrum.resize(pill_count);
		pills.resize(pill_count);
		// make sure to configure newly allocated pills!
		const auto difference = pill_count - old_pill_count;
		if (difference > 0)
			for (int i = pill_count - difference; i < pill_count; ++i)
			{
				pills[i].set_width(bar.width);
				pills[i].setPosition({float(i * (bar.width + bar.spacing)), target_size.y});
			}
	}

	fs.render(spectrum);

	for (int i = 0; i < pill_count; ++i)
	{
		// calculate new pill height (sometimes spectrum output is negative)
		auto height = multiplier * target_size.y * std::max(0.f, spectrum[i]);

		// don't go over the target's height
		height = std::min(target_size.y, height);

		pills[i].set_height(height);

		target.draw(pills[i]);
	}
}