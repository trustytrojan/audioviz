#pragma once

#include "FrequencyAnalyzer.hpp"
#include "fftwf_allocator.hpp"
#include <cassert>
#include <optional>
#include <span>
#include <vector>

namespace audioviz
{

class AudioAnalyzer
{
public:
	struct FrequencyAmplitudePair
	{
		int frequency_hz;
		float amplitude;
	};

private:
	int sample_rate_hz{};
	int fft_size{};
	int fft_output_size{}; // computed in ctor from fft_size
	std::vector<float, fftwf_allocator<float>> fft_amplitudes, fft_phase;
	std::optional<FrequencyAmplitudePair> peak_freq_amp; // {frequency_hz, amplitude}
	std::optional<std::array<FrequencyAmplitudePair, 3>> multiband_shake;

public:
	AudioAnalyzer(int sample_rate_hz, int fft_size);

	inline void set_fft_size(int fft_size)
	{
		this->fft_size = fft_size;
		fft_output_size = fft_size / 2 + 1;
	}

	void execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio, bool interleaved);
	std::span<const float> compute_amplitudes(const FrequencyAnalyzer &fa);
	std::span<const float> compute_amplitudes(const FrequencyAnalyzer &fa, int from_hz, int to_hz);
	std::span<const float> compute_phase(const FrequencyAnalyzer &fa);
	FrequencyAmplitudePair compute_peak_frequency(int from_hz, int to_hz);
	std::array<FrequencyAmplitudePair, 3> compute_multiband_shake(int from_hz, int to_hz);
};

} // namespace audioviz
