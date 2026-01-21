#include "util.hpp"

namespace avz::util
{

void resample_spectrum(
	std::span<float> spectrum,
	std::span<const float> in_amps,
	int sample_rate_hz,
	int fft_size,
	float start_freq,
	float end_freq,
	Interpolator &interpolator)
{
	interpolator.set_values(in_amps);

	const float bin_size = (float)sample_rate_hz / fft_size;
	const float bin_pos_start = (start_freq / bin_size);
	const float bin_pos_end = (end_freq / bin_size);
	const float bin_pos_step = (bin_pos_end - bin_pos_start) / std::max(1.0f, (float)spectrum.size() - 1.0f);

	float current_bin_pos = bin_pos_start;
	const auto out_size = spectrum.size();

#pragma GCC ivdep
	for (size_t i = 0; i < out_size; ++i)
	{
		spectrum[i] = interpolator.sample(current_bin_pos);
		current_bin_pos += bin_pos_step;
	}
}

void extract_channel(std::span<float> out, std::span<const float> in, int num_channels, int channel)
{
	assert(num_channels > 0);

	// out should definitely not be requesting MORE than in
	assert(out.size() * num_channels <= in.size());

	auto *__restrict const out_ptr = out.data();
	const auto *__restrict const in_ptr = in.data();

#pragma GCC ivdep
	for (size_t i = 0; i < out.size(); ++i)
		out_ptr[i] = in_ptr[i * num_channels + channel];
}

} // namespace avz
