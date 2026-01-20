#pragma once

#include <avz/Base.hpp>
#include <avz/Player.hpp>
#include <avz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <argparse/argparse.hpp>

namespace avz::examples
{

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
 * @param default_audio_duration Default FFT window duration in seconds (0.25s for FFT, smaller for oscilloscope)
 * @return Parsed configuration or exits on error
 */
ExampleConfig parse_arguments(
	int argc,
	const char *const *argv,
	const std::string &program_name,
	const std::string &description = "",
	float default_audio_duration = 0.25f);

/**
 * @brief Base class for example programs
 *
 * Provides common initialization and reduces boilerplate code
 */
class ExampleBase : public avz::Base
{
public:
	avz::FfmpegPopenMedia media;
	int sample_rate_hz;
	int num_channels;

	ExampleBase(const ExampleConfig &config);
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
template <typename VizType>
int run_example(const ExampleConfig &config, int audio_frames_needed)
{
	VizType viz{config};
	avz::Player{viz, viz.media, config.framerate, audio_frames_needed}.start_in_window(config.window_title);
	return EXIT_SUCCESS;
}

/**
 * @brief Alternative main function helper that allows custom audio frame calculation
 */
#define LIBAVZ_EXAMPLE_MAIN_CUSTOM(VizClass, description, default_audio_duration, audio_frames_expr)              \
	int main(int argc, const char *const *argv)                                                                   \
	{                                                                                                             \
		auto config = avz::examples::parse_arguments(argc, argv, #VizClass, description, default_audio_duration); \
		VizClass viz{config};                                                                                     \
		int audio_frames = (audio_frames_expr);                                                                   \
		avz::Player{viz, viz.media, config.framerate, audio_frames}.start_in_window(config.window_title);         \
		return EXIT_SUCCESS;                                                                                      \
	}

} // namespace avz::examples
