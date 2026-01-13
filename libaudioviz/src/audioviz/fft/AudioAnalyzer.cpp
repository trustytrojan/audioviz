#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/util.hpp>

namespace audioviz
{

AudioAnalyzer::AudioAnalyzer(const int num_channels)
	: _num_channels{num_channels},
	  channel_data{num_channels}
{
	assert(_num_channels);
}

void AudioAnalyzer::execute_fft(FrequencyAnalyzer &fa, const std::span<const float> audio, const bool interleaved)
{
	assert(audio.size() >= fa.get_fft_size() * _num_channels);
	for (int ch = 0; ch < _num_channels; ++ch)
	{
		fa.copy_channel_to_input(audio, _num_channels, ch, interleaved);
		fa.execute_fft();
		channel_data[ch].fft_amplitudes.resize(fa.get_fft_output_size());
		fa.compute_amplitude(channel_data[ch].fft_amplitudes);
		channel_data[ch].peaks_computed = {};
	}
}

void AudioAnalyzer::compute_peak_freq_amp(const int sample_rate_hz, const int fft_size, const float max_freq_hz)
{
	for (auto &channel : channel_data)
		channel.compute_peak_freq_amp(sample_rate_hz, fft_size, max_freq_hz);
}

std::array<AudioAnalyzer::ShakeBand, 3>
AudioAnalyzer::compute_multiband_shake(const int sample_rate_hz, const int fft_size)
{
	std::array<ShakeBand, 3> result = {};

	// Average the bands across all channels
	for (const auto &channel : channel_data)
	{
		const auto bands = channel.compute_multiband_shake(sample_rate_hz, fft_size);
		for (int i = 0; i < 3; ++i)
		{
			result[i].frequency_hz += bands[i].frequency_hz;
			result[i].amplitude += bands[i].amplitude;
		}
	}

	// Divide by number of channels to get average
	for (auto &band : result)
	{
		band.frequency_hz /= _num_channels;
		band.amplitude /= _num_channels;
	}

	return result;
}

void AudioAnalyzer::ChannelData::compute_peak_freq_amp(
	const int sample_rate_hz, const int fft_size, const float max_freq_hz)
{
	if (peaks_computed)
		return;
	const size_t max_index = max_freq_hz * fft_size / sample_rate_hz;
	const auto bass_bins = std::clamp(max_index + 1, (size_t)1, fft_amplitudes.size());
	const auto idx = util::weighted_max_index({fft_amplitudes.data(), bass_bins}, expf);
	peak_amplitude = fft_amplitudes[idx];
	peak_frequency_hz = ((float)idx * (float)sample_rate_hz) / (float)fft_size;
	peaks_computed = true;
}

std::array<AudioAnalyzer::ShakeBand, 3>
AudioAnalyzer::ChannelData::compute_multiband_shake(int sample_rate_hz, int fft_size) const
{
	// Calculate the index limit for the total bass range
	constexpr float max_freq_hz{150}; // TODO: make this configurable
	const size_t max_total_index = max_freq_hz * fft_size / sample_rate_hz;
	const auto total_bass_bins = std::clamp(max_total_index + 1, (size_t)1, fft_amplitudes.size());

	std::array<ShakeBand, 3> bands;

	// Divide the bass range into 3 equal chunks
	// (Or you can use fixed Hz ranges like 0-60, 60-120, 120-250)
	const auto chunk_size = std::max((size_t)1, total_bass_bins / 3);

	for (int i = 0; i < 3; i++)
	{
		size_t start = i * chunk_size;
		size_t end = std::min((i + 1) * chunk_size, total_bass_bins);

		const auto begin = fft_amplitudes.begin();
		const auto max_it = std::max_element(begin + start, begin + end);
		const auto index = max_it.base() - begin.base();

		bands[i].amplitude = *max_it;
		bands[i].frequency_hz = ((float)index * (float)sample_rate_hz) / (float)fft_size;
	}

	return bands;
}

std::pair<std::size_t, std::size_t> AudioAnalyzer::extract_frequency_range(
	int channel, float min_freq_hz, float max_freq_hz, int sample_rate_hz, int fft_size) const
{
	assert(channel >= 0 && channel < _num_channels);
	const auto &fft_output = channel_data[channel].fft_amplitudes;

	// Convert frequency to FFT bin indices
	size_t min_idx = static_cast<size_t>(min_freq_hz * fft_size / sample_rate_hz);
	size_t max_idx = static_cast<size_t>(max_freq_hz * fft_size / sample_rate_hz);

	// Clamp to valid range
	min_idx = std::min(min_idx, fft_output.size() - 1);
	max_idx = std::min(max_idx, fft_output.size() - 1);

	// Return the range (end_idx is inclusive)
	if (min_idx <= max_idx && min_idx < fft_output.size())
	{
		return {min_idx, max_idx};
	}

	return {0, 0};
}

} // namespace audioviz
