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

void AudioAnalyzer::execute_fft(
	FrequencyAnalyzer &fa, const std::span<const float> audio, const bool interleaved)
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

void AudioAnalyzer::ChannelData::compute_peak_freq_amp(
	const int sample_rate_hz, const int fft_size, const float max_freq_hz)
{
	if (peak_frequency_hz != -1 && peak_amplitude != -1)
		return;
	const size_t max_index = max_freq_hz * fft_size / sample_rate_hz;
	const auto bass_bins = std::clamp(max_index + 1, 1ul, fft_output.size());
	const auto idx = util::weighted_max_index({fft_output.data(), bass_bins}, expf);
	peak_amplitude = fft_output[idx];
	peak_frequency_hz = ((float)idx * (float)sample_rate_hz) / (float)fft_size;
}

} // namespace audioviz
