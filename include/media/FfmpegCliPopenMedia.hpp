#pragma once

#include "Media.hpp"

class FfmpegCliPopenMedia : public Media
{
private:
	FILE *audio{nullptr}, *video{nullptr};

public:
	FfmpegCliPopenMedia(const std::string &url, sf::Vector2u video_size = {});
	~FfmpegCliPopenMedia();

	size_t read_audio_samples(float *buf, int samples) override;
	bool read_video_frame(sf::Texture &txr) override;
};
