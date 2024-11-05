#include "media/Media.hpp"

Media::Media(const std::string &url, const sf::Vector2u video_size)
	: url{url},
	  video_size{video_size},
	  _format{url}
{
}

void Media::decode_audio(const int frames)
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

void Media::audio_buffer_erase(const int frames)
{
	const auto begin = _audio_buffer.begin();
	const auto samples = frames * _astream.nb_channels();
	_audio_buffer.erase(begin, begin + samples);
}
