#pragma once

#include "FfmpegCliMedia.hpp"
#include <boost/process.hpp>

namespace bp = boost::process;

class FfmpegCliBoostMedia : public FfmpegCliMedia
{
	bp::child audioc, videoc;
	bp::basic_pipe<float> audio;
	bp::basic_pipe<uint8_t> video;
	std::vector<uint8_t> video_buffer;

public:
	FfmpegCliBoostMedia(const std::string &url, sf::Vector2u video_size = {});
	~FfmpegCliBoostMedia();
	size_t read_audio_samples(float *buf, int samples) override;
	bool read_video_frame(sf::Texture &txr) override;
};
