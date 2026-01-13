#pragma once

#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <optional>
#include <span>
#include <vector>

namespace audioviz
{

class AudioAnalyzer_new
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
	std::vector<float> fft_amplitudes, fft_phase;
	std::optional<FrequencyAmplitudePair> peak_freq_amp; // {frequency_hz, amplitude}
	std::optional<std::array<FrequencyAmplitudePair, 3>> multiband_shake;

public:
	AudioAnalyzer_new(int sample_rate_hz, int window_size_samples);

	void execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio, bool interleaved);
	std::span<const float> compute_amplitudes(const FrequencyAnalyzer &fa);
	std::span<const float> compute_amplitudes(const FrequencyAnalyzer &fa, int from_hz, int to_hz);
	std::span<const float> compute_phase(const FrequencyAnalyzer &fa);
	FrequencyAmplitudePair compute_peak_frequency(int from_hz, int to_hz);
	std::array<FrequencyAmplitudePair, 3> compute_multiband_shake(int from_hz, int to_hz);
};

} // namespace audioviz
