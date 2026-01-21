#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <string>

namespace avz
{

class FfmpegPopenEncoder
{
	static constexpr auto NUM_PBOS = 2;

	// GLuint is just unsigned
	unsigned pbos[NUM_PBOS]{}; // this is basically a ring buffer of pixel buffer objects (PBOs) to avoid stalls
	unsigned fbo, intermediateFBO, intermediateTexture;
	int current_frame{};

	const sf::Vector2u video_size;
	const size_t byte_size{4 * video_size.x * video_size.y};
	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const std::string &media_url,
		sf::Vector2u video_size,
		int framerate,
		const std::string &outfile,
		const std::string &vcodec,
		const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const sf::Image &img);
	void send_frame(const sf::Texture &txr);
};

} // namespace avz
