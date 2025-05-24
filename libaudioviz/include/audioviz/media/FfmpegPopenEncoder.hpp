#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>
#include <audioviz/media/FfmpegEncoder.hpp>
#include <cstdio>
#include <string>

namespace audioviz::media
{

class FfmpegPopenEncoder : public FfmpegEncoder
{
private:
	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const audioviz::Base &viz, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const sf::Image &img);
	inline void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }
};

} // namespace audioviz::media
