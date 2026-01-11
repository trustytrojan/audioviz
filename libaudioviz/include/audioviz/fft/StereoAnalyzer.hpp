#pragma once

#include <audioviz/fft/AudioAnalyzer.hpp>

namespace audioviz
{

/**
 * An `AudioAnalyzer(2)` that provides getter methods `left_data()` and `right_data()`.
 */
class StereoAnalyzer : public AudioAnalyzer
{
public:
	StereoAnalyzer()
		: AudioAnalyzer(2)
	{
	}

	inline std::span<const float> left_data() const { return get_channel_data(0).fft_output; }
	inline std::span<const float> right_data() const { return get_channel_data(1).fft_output; }
};

} // namespace audioviz
