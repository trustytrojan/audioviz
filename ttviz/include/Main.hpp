#pragma once

#include "Args.hpp"
#include "ttviz.hpp"
#include "tt/FrequencyAnalyzer.hpp"
#include "viz/ColorSettings.hpp"
#include "viz/ParticleSystem.hpp"
#include "viz/ScopeDrawable.hpp"
#include "viz/SpectrumDrawable.hpp"
#include "viz/StereoSpectrum.hpp"
#include "viz/VerticalBar.hpp"

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

	void use_args(ttviz &);
	void start_in_window(audioviz &);
	void encode(audioviz &, const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
