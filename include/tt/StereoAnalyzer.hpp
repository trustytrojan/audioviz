#pragma once

#include "FrequencyAnalyzer.hpp"

namespace tt
{

/**
 * Frequency analyzer specialzed for interleaved/non-planar stereo audio.
 * Stores the resulting frequency spectrums, accessible by the `left_data`
 * and `right_data` methods.
 */
class StereoAnalyzer
{
	std::vector<float> _left, _right;

public:
	void resize(int size);
	void analyze(tt::FrequencyAnalyzer &fa, const float *stereo_audio);
	const std::vector<float> &left_data() const { return _left; }
	const std::vector<float> &right_data() const { return _right; }
};

} // namespace tt
