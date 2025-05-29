#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>
#include <audioviz/media/FfmpegEncoder.hpp>
#include <cstdio>
#include <string>

namespace audioviz
{

class FfmpegPopenEncoder : public FfmpegEncoder
{
	static const int NUM_PBOS = 1; //
	unsigned int pbos[NUM_PBOS];
	unsigned int fbo;
	unsigned int intermediateFBO;
	unsigned int intermediateTexture;
	int current_frame = 0;
	sf::Vector2u video_size;

	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const sf::Image &img);
	void send_frame(const sf::Texture &txr);
};

} // namespace audioviz::media
