#pragma once

#include "MultiChannelAudioAnalyzer.hpp"

namespace avz
{

/**
 * Stereo audio analyzer - a convenience wrapper around MultiChannelAudioAnalyzer with 2 channels.
 * Useful for stereo audio analysis.
 */
class StereoAnalyzer : public MultiChannelAudioAnalyzer
{
public:
	/**
	 * Construct stereo analyzer.
	 * @param sample_rate_hz Audio sample rate in Hz
	 * @param window_size_samples FFT window size in samples
	 */
	StereoAnalyzer()
		: MultiChannelAudioAnalyzer(2)
	{
	}

	/**
	 * Get left channel analyzer.
	 */
	AudioAnalyzer &left() { return (*this)[0]; }
	const AudioAnalyzer &left() const { return (*this)[0]; }

	/**
	 * Get right channel analyzer.
	 */
	AudioAnalyzer &right() { return (*this)[1]; }
	const AudioAnalyzer &right() const { return (*this)[1]; }
};

} // namespace avz
