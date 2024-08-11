#pragma once

#include "FrequencyAnalyzer.hpp"

namespace tt
{

/**
 * Frequency analyzer specialzed for interleaved/non-planar stereo audio.
 * Stores the resulting frequency spectrums, accessible by the `left_data`
 * and `right_data` methods.
 */
class MonoAnalyzer
{
	std::vector<float> _spectrum;

public:
	void resize(int size);
	void analyze(tt::FrequencyAnalyzer &fa, const float *audio);
	const std::vector<float> &spectrum_data() const { return _spectrum; }
};

} // namespace tt
