#include <avz/media/Media.hpp>

namespace audioviz
{

Media::Media(const std::string &url)
	: url{url}
{
}

std::optional<std::span<const float>> Media::read_audio(const int frames)
{
	const auto samples{frames * audio_channels()};
	while (_audio_buffer.size() < samples)
	{
		float buf[samples];
		const auto samples_read{read_audio_samples(buf, samples)};
		if (!samples_read)
			return {};
		_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
	}
	return {{_audio_buffer.data(), samples}};
}

void Media::consume_audio(const int frames)
{
	const auto begin{_audio_buffer.begin()};
	const auto samples{frames * audio_channels()};
	_audio_buffer.erase(begin, begin + samples);
}

} // namespace audioviz
