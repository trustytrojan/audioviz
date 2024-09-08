#pragma once

#include "Args.hpp"
#include "audioviz.hpp"
#include "viz/StereoSpectrum.hpp"
#include "tt/FrequencyAnalyzer.hpp"

#ifdef AUDIOVIZ_LUA
#include <sol/sol.hpp>
#endif

#include <boost/process.hpp>
namespace bp = boost::process;

class Main
{
	using BarType = viz::VerticalBar;
	using SD = viz::SpectrumDrawable<BarType>;
	using FS = tt::FrequencyAnalyzer;

	std::string ffmpeg_path;
	bool no_vsync = false, enc_window = false;

	tt::FrequencyAnalyzer fa{3000};
	viz::StereoSpectrum<BarType> ss;

#ifdef AUDIOVIZ_LUA
	struct LuaState : sol::state
	{
		LuaState(Main &);
	};
#endif

	struct FfmpegEncoder
	{
		bp::basic_opstream<uint8_t> std_in;
		bp::child process;
		FfmpegEncoder(audioviz &, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
		~FfmpegEncoder();
	};

	void use_args(audioviz &, const Args &);

	void start_in_window(audioviz &);
	void encode(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window_mt(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
