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
		channel_data[ch].fft_output.resize(fa.get_fft_size() / 2 + 1);
		fa.execute_fft(channel_data[ch].fft_output);
		channel_data[ch].reset_data();
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
	if (peak_frequency_hz != -1 && peak_amplitude != -1)
		return;
	const size_t max_index = max_freq_hz * fft_size / sample_rate_hz;
	const auto bass_bins = std::clamp(max_index + 1, (size_t)1, fft_output.size());
	const auto idx = util::weighted_max_index({fft_output.data(), bass_bins}, expf);
	peak_amplitude = fft_output[idx];
	peak_frequency_hz = ((float)idx * (float)sample_rate_hz) / (float)fft_size;
}

std::array<AudioAnalyzer::ShakeBand, 3>
AudioAnalyzer::ChannelData::compute_multiband_shake(int sample_rate_hz, int fft_size) const
{
	// Calculate the index limit for the total bass range
	constexpr float max_freq_hz{150}; // TODO: make this configurable
	const size_t max_total_index = max_freq_hz * fft_size / sample_rate_hz;
	const auto total_bass_bins = std::clamp(max_total_index + 1, (size_t)1, fft_output.size());

	std::array<ShakeBand, 3> bands;

	// Divide the bass range into 3 equal chunks
	// (Or you can use fixed Hz ranges like 0-60, 60-120, 120-250)
	const auto chunk_size = std::max(1ul, total_bass_bins / 3);

	for (int i = 0; i < 3; i++)
	{
		size_t start = i * chunk_size;
		size_t end = std::min((i + 1) * chunk_size, total_bass_bins);

		const auto begin = fft_output.begin();
		const auto max_it = std::max_element(begin + start, begin + end);
		const auto index = max_it.base() - begin.base();

		bands[i].amplitude = *max_it;
		bands[i].frequency_hz = ((float)index * (float)sample_rate_hz) / (float)fft_size;
	}

	return bands;
}

} // namespace audioviz
