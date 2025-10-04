#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>
#include <cstdio>
#include <string>

namespace audioviz
{

class FfmpegPopenEncoder
{
	static const int NUM_PBOS{2};

	// GLuint is just unsigned, save including header-in-header
	unsigned pbos[NUM_PBOS]{}; // this is basically a ring buffer of pixel buffer objects (PBOs) to avoid stalls
	unsigned fbo;
	unsigned intermediateFBO;
	unsigned intermediateTexture;
	int current_frame{};

	const sf::Vector2u video_size;
	const size_t byte_size{4 * video_size.x * video_size.y};
	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const std::string &media_url,
		const audioviz::Base &viz,
		const std::string &outfile,
		const std::string &vcodec,
		const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const sf::Image &img);
	void send_frame(const sf::Texture &txr);
};

} // namespace audioviz
