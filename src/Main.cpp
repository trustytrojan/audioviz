#include <GLFW/glfw3.h>

#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
	: Args(argc, argv),
	  audioviz({get<std::vector<uint>>("--size")[0],
				get<std::vector<uint>>("--size")[1]},
			   get("media_url"))
{
	use_args();
	viz_init();

	// --encode: render to video file
	switch (const auto &args = get<std::vector<std::string>>("--encode"); args.size())
	{
	case 0:
		break;
	case 1:
		encode(args[0]);
		break;
	case 2:
		encode(args[0], std::atoi(args[1].c_str()));
		break;
	case 3:
		encode(args[0], std::atoi(args[1].c_str()), args[2]);
		break;
	case 4:
		encode(args[0], std::atoi(args[1].c_str()), args[2], args[3]);
		break;
	default:
		throw std::logic_error("--encode requires 1-4 arguments");
	}

	// default behavior: render to window
	start();
}

void Main::use_args()
{
	// all of these have default values, no need to try-catch
	// set_sample_size(get<uint>("-n"));
	set_multiplier(get<float>("-m"));
	set_bar_width(get<uint>("-bw"));
	set_bar_spacing(get<uint>("-bs"));

	if (const auto ffpath = present("--ffpath"))
		ffmpeg_path = ffpath.value();

	if (const auto bg = present("--bg"))
		set_background(bg.value());

	if (const auto album_art = present("--album-art"))
		set_album_cover(album_art.value());

	if (const auto fontpath = present("--font"))
		set_text_font(*fontpath);

	// { // bar type
	// 	const auto &bt_str = get("-bt");
	// 	if (bt_str == "bar")
	// 		set_bar_type(SD::BarType::RECTANGLE);
	// 	else if (bt_str == "pill")
	// 		set_bar_type(SR::BarType::PILL);
	// 	else
	// 		throw std::invalid_argument("unknown bar type: " + bt_str);
	// }

	{ // accumulation method
		const auto &am_str = get("-a");
		if (am_str == "sum")
			set_accum_method(FS::AccumulationMethod::SUM);
		else if (am_str == "max")
			set_accum_method(FS::AccumulationMethod::MAX);
		else
			throw std::invalid_argument("unknown accumulation method: " + am_str);
	}

	{ // window function
		const auto &wf_str = get("-w");
		if (wf_str == "hanning")
			set_window_func(FS::WindowFunction::HANNING);
		else if (wf_str == "hamming")
			set_window_func(FS::WindowFunction::HAMMING);
		else if (wf_str == "blackman")
			set_window_func(FS::WindowFunction::BLACKMAN);
		else if (wf_str == "none")
			set_window_func(FS::WindowFunction::NONE);
		else
			throw std::invalid_argument("unknown window function: " + wf_str);
	}

	{ // interpolation type
		const auto &interp_str = get("-i");
		if (interp_str == "none")
			set_interp_type(FS::InterpolationType::NONE);
		else if (interp_str == "linear")
			set_interp_type(FS::InterpolationType::LINEAR);
		else if (interp_str == "cspline")
			set_interp_type(FS::InterpolationType::CSPLINE);
		else if (interp_str == "cspline_hermite")
			set_interp_type(FS::InterpolationType::CSPLINE_HERMITE);
		else
			throw std::invalid_argument("unknown interpolation type: " + interp_str);
	}

	{ // spectrum coloring type
		const auto &color_str = get("--color");
		if (color_str == "wheel")
		{
			set_color_mode(SD::ColorMode::WHEEL);
			const auto &hsv = get<std::vector<float>>("--hsv");
			assert(hsv.size() == 3);
			set_color_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			set_color_wheel_rate(get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			set_color_mode(SD::ColorMode::SOLID);
			const auto &rgb = get<std::vector<uint8_t>>("--rgb");
			set_solid_color({rgb[0], rgb[1], rgb[2]});
		}
		else
			throw std::invalid_argument("unknown coloring type: " + color_str);
	}

	// -s, --scale
	switch (const auto &scale_args = get<std::vector<std::string>>("-s"); scale_args.size())
	{
	case 0:
		break;
	case 1:
		if (scale_args[0] == "linear")
			set_scale(FS::Scale::LINEAR);
		else if (scale_args[0] == "log")
			set_scale(FS::Scale::LOG);
		else if (scale_args[0] == "nth-root")
			set_scale(FS::Scale::NTH_ROOT);
		break;
	case 2:
		if (scale_args[0] != "nth-root")
			throw std::invalid_argument("only the 'nth-root' scale takes an additional argument");
		set_nth_root(std::stoi(scale_args[1]));
	}
}

void Main::viz_init()
{
	// setting the mult for a cover art background, and applying it
	// (static backgrounds will not have effects applied automatically)
	bg.effects.emplace_back(new fx::Mult{0.75}); // TODO: automate this based on total bg luminance!!!!!!!
	bg.apply_fx();

	// change the default blur for a video (changing) background
	// bg.effects[0] = std::make_unique<fx::Blur>(5, 5, 10);
}

void Main::start()
{
	set_audio_playback_enabled(true);

	sf::RenderWindow window(sf::VideoMode(size), "audioviz", sf::Style::Titlebar, sf::State::Windowed, ctx);
	window.setVerticalSyncEnabled(true);

	{ // set framerate using display refresh rate (since we need vsync)
		const auto throwGlfwError = [](const std::string &func)
		{
			const char *errmsg;
			if (glfwGetError(&errmsg) != GLFW_NO_ERROR)
				throw std::runtime_error(func + errmsg);
		};
		if (!glfwInit())
			throwGlfwError("glfwInit");
		const auto monitor = glfwGetPrimaryMonitor();
		if (!monitor)
			throwGlfwError("glfwGetPrimaryMonitor");
		const auto mode = glfwGetVideoMode(monitor);
		if (!mode)
			throwGlfwError("glfwGetVideoMode");
		set_framerate(mode->refreshRate);
		glfwTerminate();
	}

	while (window.isOpen() && prepare_frame())
	{
		window.draw(*this);
		window.display();
		while (const auto event = window.pollEvent())
		{
			if (event.is<sf::Event::Closed>())
				window.close();
		}
		window.clear();
	}
}

void Main::ffmpeg_init(const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec)
{
	char quote;
	// clang-format off
	const auto choose_quote = [&](const std::string &s) { quote = s.contains('\'') ? '"' : '\''; };
	// clang-format on

	std::ostringstream ss;

	// hide banner, overwrite existing output file
	ss << "ffmpeg -hide_banner -y ";

	// specify input 0: raw rgba video from stdin
	ss << "-f rawvideo -pix_fmt rgba ";

	// size, framerate
	ss << "-s:v " << size.x << 'x' << size.y << " -r " << framerate << " -i - ";

	// specify input 1: media file
	choose_quote(url);
	ss << "-ss -0.1 -i " << quote << url << quote << ' ';

	// only map the audio from input 1 to the output file
	ss << "-map 0 -map 1:a ";

	if (vcodec.contains("vaapi"))
	{
		// specify the vaapi device
		ss << "-vaapi_device /dev/dri/renderD128 ";

		// convert to nv12 and upload to gpu for hardware encoding
		ss << "-vf 'format=nv12,hwupload' ";
	}

	// specify video and audio encoder
	ss << "-c:v " << vcodec << " -c:a " << acodec << ' ';

	// end output on shortest input stream
	ss << "-shortest ";

	// specify output file
	choose_quote(outfile);
	ss << quote << outfile << quote;

	const auto command = ss.str();
	std::cout << command << '\n';
	ffmpeg = popen(command.c_str(), "w");
	setbuf(ffmpeg, NULL);
}

void Main::encode(const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec)
{
	set_framerate(framerate);
	ffmpeg_init(outfile, framerate, vcodec, acodec);
	tt::RenderTexture rt(size, ctx);
	while (prepare_frame())
	{
		rt.draw(*this);
		rt.display();
		fwrite(rt.getTexture().copyToImage().getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
		rt.clear();
	}
	if (pclose(ffmpeg) == -1)
		perror("pclose");
}

void Main::encode_with_window(const std::string &outfile, int framerate, const std::string &vcodec, const std::string &acodec)
{
	set_framerate(framerate);
	ffmpeg_init(outfile, framerate, vcodec, acodec);
	sf::RenderWindow window(sf::VideoMode(size), "encoder", sf::Style::Titlebar, sf::State::Windowed, ctx);
	sf::Texture txr;
	if (!txr.create(size))
		throw std::runtime_error("failed to create texture");
	while (prepare_frame())
	{
		window.draw(*this);
		window.display();
		txr.update(window);
		fwrite(txr.copyToImage().getPixelsPtr(), 1, 4 * size.x * size.y, ffmpeg);
		window.clear();
	}
	if (pclose(ffmpeg) == -1)
		perror("pclose");
}
