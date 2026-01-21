#pragma once

#include "Interpolator.hpp"
#include <span>

namespace avz::util
{

inline constexpr int bin_index_from_freq(const int freq_hz, const int sample_rate_hz, const int bin_count)
{
	return freq_hz * bin_count / sample_rate_hz;
}

inline constexpr int freq_from_bin_index(const int bin_index, const int sample_rate_hz, const int bin_count)
{
	return bin_index * sample_rate_hz / bin_count;
}

void extract_channel(std::span<float> out, std::span<const float> in, int num_channels, int channel);

void resample_spectrum(
	std::span<float> spectrum,
	std::span<const float> in_amps,
	int sample_rate_hz,
	int fft_size,
	float start_freq,
	float end_freq,
	avz::Interpolator &interpolator);

} // namespace avz
