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
	void resize(int size)
	{
		_left.resize(size);
		_right.resize(size);
	}

	const std::vector<float> &left_data() const { return _left; }
	const std::vector<float> &right_data() const { return _right; }

	void analyze(tt::FrequencyAnalyzer &fa, const float *const stereo_audio)
	{
		// left channel
		fa.copy_channel_to_input(stereo_audio, 2, 0, true);
		fa.render(_left);

		// right channel
		fa.copy_channel_to_input(stereo_audio, 2, 1, true);
		fa.render(_right);
	}
};

} // namespace tt
