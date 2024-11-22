#pragma once

#include <SFML/Graphics.hpp>
#include <list>
#include <optional>

#include "Media.hpp"
#include "av/Frame.hpp"
#include "av/Resampler.hpp"
#include "av/SwScaler.hpp"

class LibavMedia : public Media
{
	av::Decoder _adecoder{_astream.create_decoder()};
	av::Resampler _resampler{
		// output params
		{&_astream->codecpar->ch_layout, AV_SAMPLE_FMT_FLT, _astream.sample_rate()},
		// input params
		{&_astream->codecpar->ch_layout, (AVSampleFormat)_astream->codecpar->format, _astream.sample_rate()}};
	av::Frame rs_frame;

	std::optional<av::Decoder> _vdecoder;
	std::optional<av::SwScaler> _scaler;
	std::optional<av::Frame> _scaled_frame;
	std::optional<std::list<sf::Texture>> _frame_queue;

public:
	LibavMedia(const std::string &url, sf::Vector2u vsize);
	inline size_t read_audio_samples(float *buf, int samples) override { return 0; }
	bool read_video_frame(sf::Texture &txr) override;
	void decode_audio(int audio_frames) override;
};
