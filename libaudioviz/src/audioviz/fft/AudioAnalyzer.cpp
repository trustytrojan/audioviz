#include <audioviz/fft/AudioAnalyzer.hpp>
#include <stdexcept>

namespace audioviz::fft
{

AudioAnalyzer::AudioAnalyzer(const int num_channels)
	: _num_channels{num_channels},
	  _spectrum_data_per_channel{num_channels}
{
}

void AudioAnalyzer::resize(const int size)
{
	std::ranges::for_each(_spectrum_data_per_channel, [=](auto &v) { v.resize(size); });
}

void AudioAnalyzer::analyze(FrequencyAnalyzer &fa, const float *const audio, const bool interleaved)
{
	for (int i = 0; i < _num_channels; ++i)
	{
		fa.copy_channel_to_input(audio, _num_channels, i, interleaved);
		fa.render(_spectrum_data_per_channel[i]);
	}
}

const std::vector<float> &AudioAnalyzer::get_spectrum_data(const int channel_index) const
{
	if (channel_index < 0 || channel_index >= _num_channels)
		throw std::invalid_argument("channel index out of bounds!");
	return _spectrum_data_per_channel[channel_index];
}

} // namespace audioviz::fft
