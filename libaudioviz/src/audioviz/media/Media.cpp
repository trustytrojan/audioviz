#include <audioviz/media/Media.hpp>

namespace audioviz::media
{

Media::Media(const std::string &url, const sf::Vector2u video_size)
	: url{url},
	  video_size{video_size},
	  _format{url}
{
}

void Media::audio_buffer_erase(const int frames)
{
	const auto begin = _audio_buffer.begin();
	const auto samples = frames * _astream.nb_channels();
	_audio_buffer.erase(begin, begin + samples);
}

} // namespace media
