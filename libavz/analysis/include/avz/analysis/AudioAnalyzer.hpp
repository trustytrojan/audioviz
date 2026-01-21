#pragma once

#include "FrequencyAnalyzer.hpp"
#include <cassert>
#include <span>
#include <vector>

namespace avz
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
	std::vector<float> _amplitudes;

public:
	void execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio);
	void compute_amplitudes(const FrequencyAnalyzer &fa);
	std::span<const float> get_amplitudes();
	FrequencyAmplitudePair compute_peak_frequency(const FrequencyAnalyzer &fa, int sample_rate_hz, int from_hz, int to_hz);
};

} // namespace avz
