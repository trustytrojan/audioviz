#pragma once

#include "MultiChannelAudioAnalyzer.hpp"

namespace audioviz
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
	StereoAnalyzer(int sample_rate_hz, int window_size_samples)
		: MultiChannelAudioAnalyzer(2, sample_rate_hz, window_size_samples)
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

} // namespace audioviz
