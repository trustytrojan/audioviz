#pragma once

#include "Args.hpp"
#include "audioviz.hpp"
#include "tt/FrequencyAnalyzer.hpp"
#include "viz/ColorSettings.hpp"
#include "viz/StereoSpectrum.hpp"

#ifdef AUDIOVIZ_LUA
#include <sol/sol.hpp>
#endif

#include <cstdlib>
#include <string>

class Main
{
	using BarType = viz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;

	using FA = tt::FrequencyAnalyzer;
	using SD = viz::SpectrumDrawable<BarType>;
	using SS = viz::StereoSpectrum<BarType>;
	using PS = viz::ParticleSystem<ParticleShapeType>;
	using CS = viz::ColorSettings;

	std::string ffmpeg_path;
	bool no_vsync = false, enc_window = false;

	const Args args;
	FA fa{3000};
	SS ss;
	PS ps{50};

	Main(const Main &) = delete;
	Main &operator=(const Main &) = delete;
	Main(Main &&) = delete;
	Main &operator=(Main &&) = delete;

#ifdef AUDIOVIZ_LUA
	struct LuaState : sol::state
	{
		LuaState(Main &);
	};
#endif

	class FfmpegEncoder
	{
		FILE *process;

	public:
		FfmpegEncoder(audioviz &, const std::string &outfile, const std::string &vcodec, const std::string &acodec);
		~FfmpegEncoder();
		void send_frame(const sf::Texture &);
		void send_frame(const sf::Image &);
	};

	void use_args(audioviz &);

	void start_in_window(audioviz &);
	void encode(
		audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(
		audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window_mt(
		audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(
		audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
