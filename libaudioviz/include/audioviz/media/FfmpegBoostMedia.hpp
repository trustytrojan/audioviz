#pragma once

#include <vector>

#include "Media.hpp"
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/pipe.hpp>

namespace bp = boost::process::v1;

namespace audioviz::media
{

class FfmpegBoostMedia : public Media
{
	bp::child audioc, videoc;
	// can't use basic_pipe<non-char type> because LLVM deprecated std::char_traits<non-char type>
	bp::pipe audio, video;
	std::vector<uint8_t> video_buffer;

public:
	FfmpegBoostMedia(const std::string &url, sf::Vector2u video_size = {});
	~FfmpegBoostMedia();
	size_t read_audio_samples(float *buf, int samples) override;
	bool read_video_frame(sf::Texture &txr) override;
};

} // namespace audioviz::media
