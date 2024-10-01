#pragma once

#include "Args.hpp"
#include "audioviz.hpp"
#include "viz/StereoSpectrum.hpp"
#include "tt/FrequencyAnalyzer.hpp"

#ifdef AUDIOVIZ_LUA
#include <sol/sol.hpp>
#endif

#include <string>
#include <cstdlib>

class Main
{
	using BarType = viz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;
	using SD = viz::SpectrumDrawable<BarType>;
	using FS = tt::FrequencyAnalyzer;

	std::string ffmpeg_path;
	bool no_vsync = false, enc_window = false;

	tt::FrequencyAnalyzer fa{3000};
	viz::StereoSpectrum<BarType> ss;
	viz::ParticleSystem<ParticleShapeType> ps{{{}, static_cast<sf::Vector2i>(size)}, 50};

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

	void use_args(audioviz &, const Args &);

	void start_in_window(audioviz &);
	void encode(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_without_window_mt(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");
	void encode_with_window(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
