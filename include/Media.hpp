#pragma once

#include <optional>
#include <list>
#include <SFML/Graphics.hpp>

#include "av/MediaReader.hpp"
#include "av/Resampler.hpp"
#include "av/SwScaler.hpp"
#include "av/Frame.hpp"

struct Media
{
	const std::string url;
	av::MediaReader _format;

	av::Stream _astream = _format.find_best_stream(AVMEDIA_TYPE_AUDIO);
	av::Decoder _adecoder = _astream.create_decoder();
	av::Resampler _resampler = av::Resampler(
		{&_astream->codecpar->ch_layout, AV_SAMPLE_FMT_FLT, _astream.sample_rate()},						   // output params
		{&_astream->codecpar->ch_layout, (AVSampleFormat)_astream->codecpar->format, _astream.sample_rate()}); // input params
	av::Frame rs_frame;
	std::vector<float> audio_buffer;

	std::optional<sf::Texture> attached_pic;

	std::optional<av::Stream> _vstream;
	std::optional<av::Decoder> _vdecoder;
	std::optional<av::SwScaler> _scaler;
	std::optional<av::Frame> _scaled_frame;
	std::optional<std::list<sf::Texture>> _frame_queue;

	Media(const std::string &url)
		: url(url),
		  _format(url)
	{
	}

	void init(sf::Vector2u video_frame_size);
	void decode(int audio_frames);
	void audio_buffer_erase(int frames);
	void reset();
};
