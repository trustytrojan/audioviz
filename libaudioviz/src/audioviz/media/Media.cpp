#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/media/Media.hpp>

namespace audioviz
{

Media::Media(const std::string &url)
	: url{url}
{
}

void Media::audio_buffer_erase(const int frames)
{
	const auto begin{_audio_buffer.begin()};
	const auto samples{frames * audio_channels()};
	_audio_buffer.erase(begin, begin + samples);
}

void Media::buffer_audio(const int frames)
{
	const auto samples_to_read{frames * audio_channels()};
	while (_audio_buffer.size() < samples_to_read)
	{
		float buf[samples_to_read];
		const auto samples_read{read_audio_samples(buf, samples_to_read)};
		if (!samples_read)
			return;
		_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
	}
}

} // namespace audioviz
