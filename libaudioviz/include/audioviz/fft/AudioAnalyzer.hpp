#pragma once

#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>

namespace audioviz
{

class AudioAnalyzer
{
public:
	struct ShakeBand
	{
		float frequency_hz{0.f};
		float amplitude{0.f};
	};

	struct ChannelData
	{
		std::vector<float> fft_amplitudes;
		bool peaks_computed{};
		float peak_frequency_hz, peak_amplitude;

	private:
		friend AudioAnalyzer;
		void compute_peak_freq_amp(int sample_rate_hz, int fft_size, const float max_freq_hz);
		std::array<ShakeBand, 3> compute_multiband_shake(int sample_rate_hz, int fft_size) const;
	};

	const int _num_channels;
	std::vector<ChannelData> channel_data;

public:
	AudioAnalyzer(int num_channels);

	void execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio, bool interleaved);
	void compute_peak_freq_amp(int sample_rate_hz, int fft_size, const float max_freq_hz);
	std::array<ShakeBand, 3> compute_multiband_shake(int sample_rate_hz, int fft_size);

	inline int num_channels() const { return _num_channels; }

	inline const ChannelData &get_channel_data(const int ch) const
	{
		assert(ch >= 0 && ch < _num_channels);
		return channel_data[ch];
	}

	/**
	 * Extract the FFT bin index range for a specific frequency range.
	 * @param channel Channel index
	 * @param min_freq_hz Minimum frequency (Hz)
	 * @param max_freq_hz Maximum frequency (Hz)
	 * @param sample_rate_hz Sample rate (Hz)
	 * @param fft_size FFT size
	 * @return Pair of (start_index, end_index) where end_index is inclusive; both indices are valid for the FFT output
	 */
	std::pair<std::size_t, std::size_t> extract_frequency_range(
		int channel,
		float min_freq_hz,
		float max_freq_hz,
		int sample_rate_hz,
		int fft_size) const;
};

} // namespace audioviz
