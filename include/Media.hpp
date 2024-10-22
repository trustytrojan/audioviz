#pragma once

#include <SFML/Graphics.hpp>
#include <list>
#include <optional>

#include "av/Frame.hpp"
#include "av/MediaReader.hpp"
#include "av/Resampler.hpp"
#include "av/SwScaler.hpp"

class Media
{
	const std::string url;
	av::MediaReader _format;

	av::Stream _astream{_format.find_best_stream(AVMEDIA_TYPE_AUDIO)};
	av::Decoder _adecoder{_astream.create_decoder()};
	av::Resampler _resampler{
		// output params
		{&_astream->codecpar->ch_layout, AV_SAMPLE_FMT_FLT, _astream.sample_rate()},
		// input params
		{&_astream->codecpar->ch_layout, (AVSampleFormat)_astream->codecpar->format, _astream.sample_rate()}};
	av::Frame rs_frame;
	std::vector<float> _audio_buffer;

	std::optional<sf::Texture> _attached_pic;

	std::optional<av::Stream> _vstream;
	std::optional<av::Decoder> _vdecoder;
	std::optional<av::SwScaler> _scaler;
	std::optional<av::Frame> _scaled_frame;
	std::optional<std::list<sf::Texture>> _frame_queue;

public:
	Media(const std::string &url, sf::Vector2u vsize);
	void decode_audio(int audio_frames);
	void audio_buffer_erase(int frames);

	inline const av::MediaReader &format() const { return _format; }
	inline const av::Stream &astream() const { return _astream; }
	inline const std::optional<av::Stream> &vstream() const { return _vstream; }
	inline const std::optional<sf::Texture> &attached_pic() const { return _attached_pic; }
	inline const std::vector<float> &audio_buffer() const { return _audio_buffer; }
};
