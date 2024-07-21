#include "tt/StereoAnalyzer.hpp"

namespace tt
{

void StereoAnalyzer::resize(const int size)
{
	_left.resize(size);
	_right.resize(size);
}

void StereoAnalyzer::analyze(tt::FrequencyAnalyzer &fa, const float *const stereo_audio)
{
	// left channel
	fa.copy_channel_to_input(stereo_audio, 2, 0, true);
	fa.render(_left);

	// right channel
	fa.copy_channel_to_input(stereo_audio, 2, 1, true);
	fa.render(_right);
}

} // namespace tt
