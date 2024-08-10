#include "Args.hpp"

Args::Args(const int argc, const char *const *const argv)
	: ArgumentParser(argv[0], "latest")
{
	add_argument("media_url");

	add_argument("-n", "--sample-size")
		.help("number of audio samples to process per frame\n- higher amount increases accuracy\n- lower amount increases responsiveness")
		.default_value(3000u)
		.scan<'u', uint>()
		.validate();

	add_argument("-m", "--multiplier")
		.help("spectrum amplitude multiplier")
		.default_value(4.f)
		.scan<'f', float>()
		.validate();

	add_argument("-s", "--scale")
		.help("spectrum frequency scale: 'linear', 'log', 'nth-root'\nwith 'nth-root' you can specify a floating-point root\nthe default is 2, aka sqrt")
		.nargs(1, 2)
		.default_value(std::vector<std::string>{"log"});

	add_argument("-a", "--accum-method")
		.help("frequency bin accumulation method\n- 'sum': greater treble detail, exaggerated amplitude\n- 'max': less treble detail, true-to-waveform amplitude")
		.default_value("max");

	add_argument("-w", "--window-func")
		.help("window function: 'none', 'hanning', 'hamming', 'blackman'\nwindow functions can reduce 'wiggling' in bass frequencies\nhowever they can reduce overall amplitude, so adjust '-m' accordingly")
		.default_value("blackman");

	add_argument("-i", "--interpolation")
		.help("spectrum interpolation type: 'none', 'linear', 'cspline', 'cspline_hermite'")
		.default_value("cspline");

	add_argument("--color")
		.help("enable a colorful spectrum! possible values: 'wheel', 'solid'")
		.default_value("wheel");

	add_argument("--wheel-rate")
		.help("requires '--color wheel'\nmoves the colors on the spectrum with time!\nvalue must be between [0, 1] - 0.005 is a good start")
		.default_value(0.f)
		.scan<'f', float>()
		.validate();

	add_argument("--hsv")
		.help("requires '--color wheel'\nchoose a hue offset for the color wheel, saturation, and brightness\nvalues must be between [0, 1]")
		.nargs(3)
		.default_value(std::vector<float>{0.9, 0.7, 1})
		.scan<'f', float>()
		.validate();

	add_argument("--rgb")
		.help("requires '--color solid'\nrenders the spectrum with a solid color\nmust provide space-separated rgb integers")
		.nargs(3)
		.default_value(std::vector<uint8_t>{255, 255, 255})
		.scan<'u', uint8_t>()
		.validate();

	add_argument("--size")
		.help("specify window size; args: <width> <height>")
		.default_value(std::vector<uint>{1280, 720})
		.nargs(2)
		.scan<'u', uint>()
		.validate();

	add_argument("-bw", "--bar-width")
		.help("bar width in pixels")
		.default_value(10u)
		.scan<'u', uint>()
		.validate();

	add_argument("-bs", "--bar-spacing")
		.help("bar spacing in pixels")
		.default_value(5u)
		.scan<'u', uint>()
		.validate();

	// add_argument("-bt", "--bar-type")
	// 	.help("spectrum bar style\n- 'bar': rectangular bar\n- 'pill': bar with rounded ends")
	// 	.default_value("pill");

	add_argument("--bg")
		.help("add a background image; path to image file required");

	add_argument("--font")
		.help("metadata text font; must be an absolute path");
	
	add_argument("--album-art")
		.help("render an album-art image next to the song metadata\npath to image file required");

	add_argument("-bm", "--blendmode")
		.help("color blend mode for the spectrum; see --bm-help for help")
		.nargs(0, 6)
		.validate();

	add_argument("--bm-help")
		.help("show help for --blendmode")
		.flag();
	
	add_argument("--encode")
		.help("encode to a video file using ffmpeg! arguments: <output_file> [fps=60] [vcodec=h264] [acodec=copy]")
		.nargs(1, 4)
		.validate();
	
	add_argument("--enc-window")
		.help("when used with --encode, renders the current frame being encoded to a window")
		.flag();

	add_argument("--ffpath")
		.help("specify path to ffmpeg executable used by '--encode'");
	
	add_argument("-r", "--framerate")
		.help("set visualizer framerate; spectrum analysis depends on this so setting it too high will backfire!")
		.default_value(60u)
		.scan<'u', uint>()
		.validate();

	add_argument("--no-vsync")
		.help("disable vsync (not recommended)")
		.flag();
	
	add_argument("--luafile")
		.help("NEW FEATURE!!!!!! configure audioviz with a lua script!\nlua code is run BEFORE cli args are parsed!");

	add_argument("--no-fx")
		.help("don't add default effects")
		.flag();

	try
	{
		parse_args(argc, argv);

		if (get<bool>("--bm-help"))
		{
			std::cout << R"(possible argument cases:
- 1 arg: 'alpha', 'add', 'mult', 'min', 'max', 'none'
- 3 args: <srcfactor> <dstfactor> <op>
- 6 args: <color_sf> <color_df> <op> <alpha_sf> <alpha_df> <op>
  - factors can be one of:
    - '0': zero
    - '1': one
    - 'sc': source color
    - '1sc': 1 minus source color
    - 'dc': destination color
    - '1dc': 1 minus destination color
    - 'sa': source alpha
    - '1sa': 1 minus source alpha
    - 'da': destination alpha
    - '1da': 1 minus destination alpha
  - operations can be one of:
    - 'add'
    - 'sub': subtract
    - 'rs': reverse subtract (flips the sf/df order before subtracting)
    - 'min'
    - 'max'
)";
			_Exit(EXIT_SUCCESS);
		}
	}
	catch (const std::exception &e)
	{
		// print error and help to stderr
		std::cerr << argv[0] << ": " << e.what() << '\n';

		// just exit here since we don't want to print anything after the help
		_Exit(EXIT_FAILURE);
	}
}