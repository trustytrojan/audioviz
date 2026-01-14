#include <audioviz/fft/MultiChannelAudioAnalyzer.hpp>

#include <stdexcept>

namespace audioviz
{

MultiChannelAudioAnalyzer::MultiChannelAudioAnalyzer(int num_channels, int sample_rate_hz, int window_size_samples)
	: num_channels{num_channels}
{
	if (num_channels <= 0)
		throw std::invalid_argument("num_channels must be > 0");

	analyzers.reserve(num_channels);
	for (int i = 0; i < num_channels; ++i)
		analyzers.emplace_back(sample_rate_hz, window_size_samples);
}

void MultiChannelAudioAnalyzer::execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio)
{
	const int window_size = fa.get_fft_size();
	if (audio.size() < window_size * num_channels)
		throw std::invalid_argument("Audio buffer too small for interleaved data");

	// Extract each channel and run FFT
	for (int ch = 0; ch < num_channels; ++ch)
	{
		// Copy channel data (deinterleave)
		std::vector<float> channel_audio(window_size);
		for (int i = 0; i < window_size; ++i)
			channel_audio[i] = audio[i * num_channels + ch];

		analyzers[ch].execute_fft(fa, channel_audio, false);
	}
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

AudioAnalyzer::FrequencyAmplitudePair
MultiChannelAudioAnalyzer::compute_averaged_peak_frequency(int from_hz, int to_hz)
{
	float total_freq = 0.f;
	float total_amp = 0.f;

	for (auto &analyzer : analyzers)
	{
		auto [freq, amp] = analyzer.compute_peak_frequency(from_hz, to_hz);
		total_freq += freq;
		total_amp += amp;
	}

	return {static_cast<int>(total_freq / num_channels), total_amp / num_channels};
}

std::array<AudioAnalyzer::FrequencyAmplitudePair, 3>
MultiChannelAudioAnalyzer::compute_averaged_multiband_shake(int from_hz, int to_hz)
{
	std::array<AudioAnalyzer::FrequencyAmplitudePair, 3> result{};

	for (auto &analyzer : analyzers)
	{
		auto bands = analyzer.compute_multiband_shake(from_hz, to_hz);
		for (int i = 0; i < 3; ++i)
		{
			result[i].frequency_hz += bands[i].frequency_hz;
			result[i].amplitude += bands[i].amplitude;
		}
	}

	for (auto &band : result)
	{
		band.frequency_hz /= num_channels;
		band.amplitude /= num_channels;
	}

	return result;
}

} // namespace audioviz
