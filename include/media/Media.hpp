#pragma once

#include <SFML/Graphics.hpp>
#include <av/MediaReader.hpp>

class Media
{
public:
	const std::string url;
	const sf::Vector2u video_size;

protected:
	av::MediaReader _format;

	av::Stream _astream{_format.find_best_stream(AVMEDIA_TYPE_AUDIO)};
	std::vector<float> _audio_buffer;

	std::optional<av::Stream> _vstream;
	std::optional<sf::Texture> _attached_pic;

public:
	Media(const std::string &url, sf::Vector2u video_size);

	virtual size_t read_audio_samples(float *buf, int samples) = 0;
	virtual bool read_video_frame(sf::Texture &txr) = 0;
	virtual void decode_audio(int frames) = 0;
	void audio_buffer_erase(int frames);

	inline const av::MediaReader &format() const { return _format; }
	inline const av::Stream &astream() const { return _astream; }
	inline const std::optional<av::Stream> &vstream() const { return _vstream; }
	inline const std::optional<sf::Texture> &attached_pic() const { return _attached_pic; }
	inline const std::vector<float> &audio_buffer() const { return _audio_buffer; }
};
