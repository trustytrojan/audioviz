#pragma once

#include "AudioAnalyzer.hpp"
#include <array>
#include <span>
#include <vector>

namespace audioviz
{

/**
 * Multi-channel audio analyzer that manages separate AudioAnalyzer instances per channel.
 * Useful for stereo/multi-channel audio analysis.
 */
class MultiChannelAudioAnalyzer
{
	int num_channels{};
	std::vector<AudioAnalyzer> analyzers;

public:
	/**
	 * Construct analyzer for N channels.
	 * @param num_channels Number of audio channels (1=mono, 2=stereo, etc.)
	 * @param sample_rate_hz Audio sample rate in Hz
	 * @param window_size_samples FFT window size in samples
	 */
	MultiChannelAudioAnalyzer(int num_channels, int sample_rate_hz, int window_size_samples);

	/**
	 * Execute FFT on interleaved multi-channel audio buffer.
	 * Automatically distributes audio to per-channel analyzers.
	 * @param fa FrequencyAnalyzer (shared across channels)
	 * @param interleaved_audio Interleaved audio buffer (size = window_size * num_channels)
	 */
	void execute_fft(FrequencyAnalyzer &fa, std::span<const float> interleaved_audio);

	/**
	 * Get analyzer for specific channel.
	 * @param channel Channel index (0-based)
	 */
	AudioAnalyzer &operator[](int channel);
	const AudioAnalyzer &operator[](int channel) const;

	/**
	 * Get number of channels.
	 */
	int get_num_channels() const { return num_channels; }

	/**
	 * Average peak frequency across all channels (useful for stereo->mono reduction).
	 */
	AudioAnalyzer::FrequencyAmplitudePair compute_averaged_peak_frequency(int from_hz, int to_hz);

	/**
	 * Average multiband shake across all channels.
	 */
	std::array<AudioAnalyzer::FrequencyAmplitudePair, 3> compute_averaged_multiband_shake(int from_hz, int to_hz);
};

} // namespace audioviz
