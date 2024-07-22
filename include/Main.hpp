#pragma once

#include "Args.hpp"
#include "audioviz.hpp"

class Main : Args, public audioviz
{
	static inline const sf::ContextSettings ctx{0, 0, 4};

	std::string ffmpeg_path;
	FILE *ffmpeg = nullptr;
	
	void ffmpeg_init(const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec);
	void viz_init();
	void use_args();

public:
	Main(const int argc, const char *const *const argv);
	void start();
	void encode(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
};