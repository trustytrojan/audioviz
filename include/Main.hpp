#pragma once

#include "Args.hpp"
#include "audioviz.hpp"
#include <sol.hpp>

class Main
{
	using BarType = viz::VerticalBar;
	using SD = viz::SpectrumDrawable<BarType>;
	using FS = tt::FrequencyAnalyzer;

	static inline const sf::ContextSettings ctx{0, 0, 4};
	std::shared_ptr<audioviz> viz;

	std::string ffmpeg_path;
	FILE *ffmpeg = nullptr;
	bool no_vsync = false, enc_window = false;
	
	sol::state lua_init();
	void ffmpeg_init(const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec);
	void use_args(const Args &);

public:
	Main(const int argc, const char *const *const argv);
	void start();
	void encode(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
};