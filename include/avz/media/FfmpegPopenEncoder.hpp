#pragma once

#include <cstdio>
#include <string>

namespace avz
{

class FfmpegPopenEncoder
{
	static constexpr auto NUM_PBOS{2};

	// GLuint is just unsigned
	unsigned pbos[NUM_PBOS]{}; // this is basically a ring buffer of pixel buffer objects (PBOs) to avoid stalls
	unsigned fbo, intermediateFBO, intermediateTexture;
	int current_frame{};

	const unsigned video_width, video_height;
	const size_t byte_size{4 * video_width * video_height};
	FILE *ffmpeg;

public:
	FfmpegPopenEncoder(
		const std::string &media_url,
		unsigned video_width,
		unsigned video_height,
		int framerate,
		const std::string &outfile,
		const std::string &vcodec,
		const std::string &acodec);
	~FfmpegPopenEncoder();

	void send_frame(const unsigned glTexture);
};

} // namespace avz
