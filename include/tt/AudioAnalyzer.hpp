#pragma once

#include "FrequencyAnalyzer.hpp"

namespace tt
{

class AudioAnalyzer
{
private:
	int _num_channels;
	std::vector<std::vector<float>> _spectrum_data_per_channel;

public:
	AudioAnalyzer(int num_channels);
	AudioAnalyzer(const AudioAnalyzer &) = delete;
	AudioAnalyzer &operator=(const AudioAnalyzer &) = delete;
	AudioAnalyzer(AudioAnalyzer &&) = delete;
	AudioAnalyzer &operator=(AudioAnalyzer &&) = delete;
	void resize(int size);

	/**
	 * Analyze interleaved 32-bit floating point audio.
	 * Remember that interleaved means the samples are arranged
	 * such that `audio[0]` belongs to the first channel, `audio[1]`
	 * the second, and so on until `audio[num_channels - 1]`. Then
	 * the pattern repeats.
	 */
	void analyze(tt::FrequencyAnalyzer &fa, const float *audio, bool interleaved);

	int get_num_channels() const;
	const std::vector<float> &get_spectrum_data(int channel_index) const;
};

} // namespace tt
