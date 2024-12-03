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
	using ShapeType = sf::RectangleShape;

	using FA = tt::FrequencyAnalyzer;
	using SC = viz::ScopeDrawable<ShapeType>;
	using SD = viz::SpectrumDrawable<BarType>;
	using SS = viz::StereoSpectrum<BarType>;
	using PS = viz::ParticleSystem<ParticleShapeType>;
	using CS = viz::ColorSettings;

	std::string ffmpeg_path;
	bool no_vsync = false, enc_window = false;

	const Args args;
	FA fa{3000};
	CS cs;
	SS ss{cs};
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

	void use_args(audioviz &);
	void start_in_window(base_audioviz &);
	void encode(base_audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
