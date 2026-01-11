#pragma once

#include <audioviz/fft/FrequencyAnalyzer.hpp>

namespace audioviz
{

class AudioAnalyzer
{
	struct ChannelData
	{
		std::vector<float> fft_output;
		float peak_frequency_hz{-1}, peak_amplitude{-1};

	private:
		friend AudioAnalyzer;
		void reset_data() { peak_frequency_hz = peak_amplitude = -1; }
		void compute_peak_freq_amp(int sample_rate_hz, int fft_size, const float max_freq_hz);
	};

	const int _num_channels;
	std::vector<ChannelData> channel_data;

public:
	AudioAnalyzer(int num_channels);

	void execute_fft(
		FrequencyAnalyzer &fa, std::span<const float> audio, bool interleaved);
	void compute_peak_freq_amp(int sample_rate_hz, int fft_size, const float max_freq_hz);

	inline int num_channels() const { return _num_channels; }

	inline const ChannelData &get_channel_data(const int ch) const
	{
		assert(ch >= 0 && ch < _num_channels);
		return channel_data[ch];
	}
};

} // namespace audioviz
