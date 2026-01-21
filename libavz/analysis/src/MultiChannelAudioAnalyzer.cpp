#include <avz/analysis/util.hpp>
#include <avz/analysis/MultiChannelAudioAnalyzer.hpp>

#include <stdexcept>

namespace avz
{

MultiChannelAudioAnalyzer::MultiChannelAudioAnalyzer(int num_channels)
	: num_channels{num_channels}
{
	if (num_channels <= 0)
		throw std::invalid_argument("num_channels must be > 0");

	analyzers.resize(num_channels);
}

void MultiChannelAudioAnalyzer::execute_fft(FrequencyAnalyzer &fa, std::span<const float> interleaved_audio)
{
	const int window_size = fa.get_fft_size();
	assert(interleaved_audio.size() == window_size * num_channels);

	// Extract each channel and run FFT
	for (int ch = 0; ch < num_channels; ++ch)
	{
		// Copy channel data (deinterleave)
		float buf[window_size];
		std::span channel_audio{buf, static_cast<size_t>(window_size)};
		util::extract_channel(channel_audio, interleaved_audio, num_channels, ch);
		analyzers[ch].execute_fft(fa, channel_audio);
	}
}

void MultiChannelAudioAnalyzer::compute_amplitudes(FrequencyAnalyzer &fa)
{
	for (int ch = 0; ch < num_channels; ++ch)
		analyzers[ch].compute_amplitudes(fa);
}

AudioAnalyzer &MultiChannelAudioAnalyzer::operator[](int channel)
{
	if (channel < 0 || channel >= num_channels)
		throw std::out_of_range("Invalid channel index");
	return analyzers[channel];
}

const AudioAnalyzer &MultiChannelAudioAnalyzer::operator[](int channel) const
{
	if (channel < 0 || channel >= num_channels)
		throw std::out_of_range("Invalid channel index");
	return analyzers[channel];
}

AudioAnalyzer::FrequencyAmplitudePair MultiChannelAudioAnalyzer::compute_averaged_peak_frequency(
	const FrequencyAnalyzer &fa, int sample_rate_hz, int from_hz, int to_hz)
{
	float total_freq = 0.f;
	float total_amp = 0.f;

	for (auto &analyzer : analyzers)
	{
		auto [freq, amp] = analyzer.compute_peak_frequency(fa, sample_rate_hz, from_hz, to_hz);
		total_freq += freq;
		total_amp += amp;
	}

	return {static_cast<int>(total_freq / num_channels), total_amp / num_channels};
}

} // namespace avz
