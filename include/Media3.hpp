#pragma once

#include <SFML/Graphics.hpp>
#include <av/MediaReader.hpp>

class Media3
{
	std::string url;
	sf::Vector2u video_size;
	FILE *audio{nullptr}, *video{nullptr};
	std::string audio_cmd, video_cmd;
	av::MediaReader _format;
	av::Stream _astream{_format.find_best_stream(AVMEDIA_TYPE_AUDIO)};
	std::optional<av::Stream> _vstream;
	std::optional<sf::Texture> _attached_pic;
	std::vector<float> _audio_buffer;

public:
	Media3(const std::string &url, sf::Vector2u video_size = {});
	~Media3();
	inline const av::MediaReader &format() const { return _format; }
	inline const av::Stream &astream() const { return _astream; }
	inline const std::optional<av::Stream> &vstream() const { return _vstream; }
	inline const std::optional<sf::Texture> &attached_pic() const { return _attached_pic; }
	inline const std::vector<float> &audio_buffer() const { return _audio_buffer; }
	size_t read_audio_samples(float *buf, int samples) const;
	bool read_video_frame(sf::Texture &txr) const;
	void decode_audio(int frames);
	void audio_buffer_erase(int frames);
	void reset();
};
