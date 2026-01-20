#include "ExampleFramework.hpp"

namespace avz::examples
{

ExampleConfig
parse_arguments(int argc, const char *const *argv, const std::string &program_name, const std::string &description)
{
	argparse::ArgumentParser parser(program_name);

	if (!description.empty())
		parser.add_description(description);

	// clang-format off
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
	// clang-format on

	try
	{
		parser.parse_args(argc, argv);
	}
	catch (const std::exception &err)
	{
		std::cerr << err.what() << std::endl;
		std::cerr << parser;
		std::exit(EXIT_FAILURE);
	}

	ExampleConfig config;
	config.media_path = parser.get<std::string>("media");

	auto size_values = parser.get<std::vector<unsigned>>("--size");
	if (size_values.size() != 2)
		size_values = {1280, 720};
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

} // namespace avz::examples
