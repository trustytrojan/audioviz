#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv) :
	Args(argc, argv),
	audioviz({get<uint>("--width"), get<uint>("--height")}, get("audio_file"))
{
	// all of these have default values, no need to try-catch
	// set_sample_size(get<uint>("-n"));
	set_multiplier(get<float>("-m"));
	set_bar_width(get<uint>("-bw"));
	set_bar_spacing(get<uint>("-bs"));
	// set_mono(get<int>("--mono"));

	// finally i realized what `present` does
	// no need to try-catch on `get` anymore...

	if (const auto ffpath = present("--ffmpeg-path"))
		ffmpeg_path = ffpath.value();

	if (const auto bg = present("--bg"))
		set_background(bg.value());

	if (const auto album_art = present("--album-art"))
		set_album_cover(album_art.value());

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

	// --encode (decides whether we render to the window or to a video)
	switch (const auto &encode_args = get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		start();
		break;
		// case 2:
		// 	encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()));
		// 	break;
		// case 3:
		// 	encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()), encode_args[2]);
		// 	break;
		// case 4:
		// 	encode_to_video(encode_args[0], std::atoi(encode_args[1].c_str()), encode_args[2], encode_args[3]);
		// 	break;
	default:
		throw std::logic_error("--encode should only have 2-4 arguments");
	}
}

#include <GLFW/glfw3.h>

void Main::start()
{
	set_audio_playback_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
	set_metadata_position({30, 30});

	sf::RenderWindow window(sf::VideoMode(size), "audioviz", sf::Style::Titlebar, sf::State::Windowed, sf::ContextSettings{0, 0, 4});
	window.setVerticalSyncEnabled(true);

	{ // set framerate using display refresh rate (since we need vsync)
		const auto throwGlfwError = [](const std::string &func) {
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

	// setting the mult for a cover art background, and applying it
	// (static backgrounds will not have effects applied automatically)
	bg.effects.emplace_back(new fx::Mult{0.75}); // TODO: automate this based on total bg luminance!!!!!!!
	bg.apply_fx();

	// change the default blur for a video (changing) background
	// viz.bg.effects[0] = std::make_unique<fx::Blur>(5, 5, 10);

	while (window.isOpen() && prepare_frame())
	{
		window.draw(*this);
		window.display();
		while (const auto event = window.pollEvent())
		{
			if (event.is<sf::Event::Closed>())
				window.close();
		}
	}
}