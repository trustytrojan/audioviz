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

	std::string ffmpeg_path;
	bool no_vsync = false, enc_window = false;

	struct LuaState : sol::state
	{
		LuaState(Main &);
	};

	FILE *ffmpeg_open(audioviz &, const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec);
	void use_args(audioviz &, const Args &);

	void start_in_window(audioviz &);
	void encode(audioviz &, const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(audioviz &, const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window_mt(audioviz &, const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(audioviz &, const std::string &outfile, int framerate = 60, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};