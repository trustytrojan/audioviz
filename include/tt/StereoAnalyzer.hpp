#pragma once

#include "AudioAnalyzer.hpp"

namespace tt
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

	const std::vector<float> &left_data() const { return get_spectrum_data(0); }
	const std::vector<float> &right_data() const { return get_spectrum_data(1); }
};

} // namespace tt
