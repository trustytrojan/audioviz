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
	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const sf::Image &img);
	inline void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }
};

} // namespace audioviz
