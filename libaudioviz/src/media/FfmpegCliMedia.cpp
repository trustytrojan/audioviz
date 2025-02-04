#include "media/FfmpegCliMedia.hpp"

namespace media
{

void FfmpegCliMedia::decode_audio(const int frames)
{
	const auto samples_to_read = frames * _astream.nb_channels();
	while (_audio_buffer.size() < samples_to_read)
	{
		float buf[samples_to_read];
		const auto samples_read = read_audio_samples(buf, samples_to_read);
		if (!samples_read)
			return;
		_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
	}
}

} // namespace media
