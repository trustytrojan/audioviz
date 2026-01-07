#pragma once

#include "Args.hpp"
#include "ttviz.hpp"
#include <audioviz/ColorSettings.hpp>
#include <audioviz/ParticleSystem.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/StereoSpectrum.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>

#include <string>

class Main
{
	using BarType = audioviz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;
	using ShapeType = sf::RectangleShape;

	using FA = audioviz::FrequencyAnalyzer;
	using SC = audioviz::ScopeDrawable<ShapeType>;
	using SD = audioviz::SpectrumDrawable<BarType>;
	using SS = audioviz::StereoSpectrum<BarType>;
	using PS = audioviz::ParticleSystem<ParticleShapeType>;
	using CS = audioviz::ColorSettings;

	std::string ffmpeg_path;
	bool no_vsync = false;

	const Args args;
	std::unique_ptr<audioviz::Media> media;
	FA fa{3000};
	CS cs;
	SS ss{cs};
	PS ps{{}, 50};

	Main(const Main &) = delete;
	Main &operator=(const Main &) = delete;
	Main(Main &&) = delete;
	Main &operator=(Main &&) = delete;

	void use_args(ttviz &);
	void start_in_window(audioviz::Base &);
	void encode(
		audioviz::Base &,
		const std::string &outfile,
		const std::string &vcodec = "h264",
		const std::string &acodec = "copy");

public:
	Main(const int argc, const char *const *const argv);
};
