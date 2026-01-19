#pragma once

#include <audioviz/Base.hpp>
#include <audioviz/Player.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <argparse/argparse.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>

namespace audioviz::examples {

/**
 * @brief Configuration structure for example programs
 * 
 * Contains common parameters that can be configured via command-line arguments
 */
struct ExampleConfig
{
	sf::Vector2u size{1280, 720};
	std::string media_path;
	int framerate = 60;
	float audio_duration_sec = 0.25f;
	float media_start_time_sec = 0.0f;
	bool profiler_enabled = false;
	std::string font_path;
	std::string window_title;
};

/**
 * @brief Parse command-line arguments for example programs
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @param program_name Name of the program for help messages
 * @param description Optional program description
 * @return Parsed configuration or exits on error
 */
inline ExampleConfig parse_arguments(
	int argc,
	const char* const* argv,
	const std::string& program_name,
	const std::string& description = "")
{
	argparse::ArgumentParser parser(program_name);
	
	if (!description.empty())
	{
		parser.add_description(description);
	}

	// Required positional argument: media file
	parser.add_argument("media")
		.help("Path to the media file (audio/video)");

	// Optional arguments with sensible defaults
	parser.add_argument("-s", "--size")
		.help("Window size as width height (pixels)")
		.nargs(2)
		.scan<'d', unsigned>();

	parser.add_argument("-f", "--framerate")
		.help("Output framerate (FPS)")
		.default_value(60)
		.scan<'d', int>();

	parser.add_argument("--fft-window")
		.help("FFT window duration (seconds)")
		.default_value(0.25f)
		.scan<'g', float>();

	parser.add_argument("--media-start")
		.help("Media seek position (seconds)")
		.default_value(0.0f)
		.scan<'g', float>();

	parser.add_argument("-p", "--profiler")
		.help("Enable performance profiler")
		.flag();

	parser.add_argument("--font")
		.help("Path to font file for profiler")
		.default_value("");

	try
	{
		parser.parse_args(argc, argv);
	}
	catch (const std::exception& err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << parser;
		std::exit(EXIT_FAILURE);
	}

	ExampleConfig config;
	config.media_path = parser.get<std::string>("media");
	
	auto size_values = parser.get<std::vector<unsigned>>("--size");
	if (size_values.size() != 2)
	{
		size_values = {1280, 720};
	}
	config.size.x = size_values[0];
	config.size.y = size_values[1];
	
	config.framerate = parser.get<int>("--framerate");
	config.audio_duration_sec = parser.get<float>("--fft-window");
	config.media_start_time_sec = parser.get<float>("--media-start");
	config.profiler_enabled = parser.get<bool>("--profiler");
	config.font_path = parser.get<std::string>("--font");
	config.window_title = argv[0];
	
	// Validate values
	if (config.size.x <= 0 || config.size.y <= 0)
	{
		std::cerr << "Error: Window dimensions must be positive\n";
		std::exit(EXIT_FAILURE);
	}
	if (config.framerate <= 0)
	{
		std::cerr << "Error: Framerate must be positive\n";
		std::exit(EXIT_FAILURE);
	}
	if (config.audio_duration_sec <= 0.0f)
	{
		std::cerr << "Error: FFT window duration must be positive\n";
		std::exit(EXIT_FAILURE);
	}
	if (config.media_start_time_sec < 0.0f)
	{
		std::cerr << "Error: Media start time cannot be negative\n";
		std::exit(EXIT_FAILURE);
	}

	return config;
}

/**
 * @brief Base class template for example programs
 * 
 * Provides common initialization and reduces boilerplate code
 * 
 * @tparam Derived The derived example class (CRTP pattern)
 */
template<typename Derived>
class ExampleBase : public audioviz::Base
{
public:
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz;
	int num_channels;

	ExampleBase(const ExampleConfig& config)
		: Base{config.size},
		  media{config.media_path, config.media_start_time_sec},
		  sample_rate_hz{media.audio_sample_rate()},
		  num_channels{media.audio_channels()}
	{
		if (config.profiler_enabled)
		{
			enable_profiler();
			if (config.font_path.size())
				set_font(config.font_path);
			else
			 	std::cerr << "profiler enabled but no font file provided, profiler text will not be visible\n";
		}
	}

	virtual ~ExampleBase() = default;
};

/**
 * @brief Run an example visualization
 * 
 * This function encapsulates the common pattern of creating a visualization
 * and running it with a Player.
 * 
 * @tparam VizType The visualization class derived from ExampleBase
 * @param config Configuration for the example
 * @param audio_frames_needed Number of audio frames needed per update
 * @return Exit code (0 for success)
 */
template<typename VizType>
int run_example(const ExampleConfig& config, int audio_frames_needed)
{
	VizType viz{config};
	audioviz::Player{viz, viz.media, config.framerate, audio_frames_needed}
		.start_in_window(config.window_title);
	return EXIT_SUCCESS;
}

/**
 * @brief Convenience macro for main function boilerplate
 * 
 * Usage:
 * AUDIOVIZ_EXAMPLE_MAIN(MyVisualization, "Description of my visualization")
 */
#define AUDIOVIZ_EXAMPLE_MAIN(VizClass, description)                                     \
	int main(int argc, const char* const* argv)                                          \
	{                                                                                     \
		auto config = audioviz::examples::parse_arguments(argc, argv, #VizClass, description); \
		VizClass viz{config};                                                             \
		return audioviz::examples::run_example<VizClass>(config, viz.get_audio_frames_needed()); \
	}

/**
 * @brief Alternative main function helper that allows custom audio frame calculation
 */
#define AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(VizClass, description, audio_frames_expr)           \
	int main(int argc, const char* const* argv)                                          \
	{                                                                                     \
		auto config = audioviz::examples::parse_arguments(argc, argv, #VizClass, description); \
		VizClass viz{config};                                                             \
		int audio_frames = (audio_frames_expr);                                           \
		audioviz::Player{viz, viz.media, config.framerate, audio_frames}                 \
			.start_in_window(config.window_title);                                        \
		return EXIT_SUCCESS;                                                              \
	}

} // namespace audioviz::examples
