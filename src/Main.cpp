#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
{
	Args args(argc, argv);

#ifdef AUDIOVIZ_LUA
	// this is a hack...
	argparse::ArgumentParser parser("audioviz");
	parser.add_argument("--luafile");
	if (const auto luafile = args.present("--luafile"))
	{
		LuaState(*this).do_file(*luafile);
		return;
		// lua environment is still in the works!!!!!!!!!
	}
#endif

	const auto &size = args.get<std::vector<uint>>("--size");
	audioviz viz(sf::Vector2u{size[0], size[1]}, args.get("media_url"));
	use_args(viz, args);

	// --encode: render to video file
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		// default behavior: render to window
		start_in_window(viz);
		break;
	case 1:
		encode(viz, encode_args[0]);
		break;
	case 2:
		encode(viz, encode_args[0], encode_args[1]);
		break;
	case 3:
		encode(viz, encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error("--encode requires 1-3 arguments");
	}
}

void Main::use_args(audioviz &viz, const Args &args)
{
	// default-value params
	viz.set_sample_size(args.get<uint>("-n"));
	viz.set_multiplier(args.get<float>("-m"));
	viz.set_bar_width(args.get<uint>("-bw"));
	viz.set_bar_spacing(args.get<uint>("-bs"));
	viz.set_framerate(args.get<uint>("-r"));
	no_vsync = args.get<bool>("--no-vsync");
	enc_window = args.get<bool>("--enc-window");

	// no-default-value params
	if (const auto ffpath = args.present("--ffpath"))
		ffmpeg_path = ffpath.value();

	if (const auto bg = args.present("--bg"))
		viz.set_background(bg.value());

	if (const auto album_art = args.present("--album-art"))
		viz.set_album_cover(album_art.value());

	if (const auto fontpath = args.present("--font"))
		viz.set_text_font(*fontpath);

	if (!args.get<bool>("--no-fx"))
		viz.add_default_effects();

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
		const auto &am_str = args.get("-a");
		if (am_str == "sum")
			viz.set_accum_method(FS::AccumulationMethod::SUM);
		else if (am_str == "max")
			viz.set_accum_method(FS::AccumulationMethod::MAX);
		else
			throw std::invalid_argument("unknown accumulation method: " + am_str);
	}

	{ // window function
		const auto &wf_str = args.get("-w");
		if (wf_str == "hanning")
			viz.set_window_func(FS::WindowFunction::HANNING);
		else if (wf_str == "hamming")
			viz.set_window_func(FS::WindowFunction::HAMMING);
		else if (wf_str == "blackman")
			viz.set_window_func(FS::WindowFunction::BLACKMAN);
		else if (wf_str == "none")
			viz.set_window_func(FS::WindowFunction::NONE);
		else
			throw std::invalid_argument("unknown window function: " + wf_str);
	}

	{ // interpolation type
		const auto &interp_str = args.get("-i");
		if (interp_str == "none")
			viz.set_interp_type(FS::InterpolationType::NONE);
		else if (interp_str == "linear")
			viz.set_interp_type(FS::InterpolationType::LINEAR);
		else if (interp_str == "cspline")
			viz.set_interp_type(FS::InterpolationType::CSPLINE);
		else if (interp_str == "cspline_hermite")
			viz.set_interp_type(FS::InterpolationType::CSPLINE_HERMITE);
		else
			throw std::invalid_argument("unknown interpolation type: " + interp_str);
	}

	{ // spectrum coloring type
		const auto &color_str = args.get("--color");
		if (color_str == "wheel")
		{
			viz.set_color_mode(SD::ColorMode::WHEEL);
			const auto &hsv = args.get<std::vector<float>>("--hsv");
			assert(hsv.size() == 3);
			viz.set_color_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			viz.set_color_wheel_rate(args.get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			viz.set_color_mode(SD::ColorMode::SOLID);
			const auto &rgb = args.get<std::vector<uint8_t>>("--rgb");
			viz.set_solid_color({rgb[0], rgb[1], rgb[2]});
		}
		else
			throw std::invalid_argument("unknown coloring type: " + color_str);
	}

	{ // spectrum blendmode
		static const std::unordered_map<std::string, sf::BlendMode::Factor> factor_map{
			{"0", sf::BlendMode::Factor::Zero},
			{"1", sf::BlendMode::Factor::One},
			{"sc", sf::BlendMode::Factor::SrcColor},
			{"1sc", sf::BlendMode::Factor::OneMinusSrcColor},
			{"dc", sf::BlendMode::Factor::DstColor},
			{"1dc", sf::BlendMode::Factor::OneMinusDstColor},
			{"sa", sf::BlendMode::Factor::SrcAlpha},
			{"1sa", sf::BlendMode::Factor::OneMinusSrcAlpha},
			{"da", sf::BlendMode::Factor::DstAlpha},
			{"1da", sf::BlendMode::Factor::OneMinusDstAlpha}};

		static const std::unordered_map<std::string, sf::BlendMode::Equation> op_map{
			{"add", sf::BlendMode::Equation::Add},
			{"sub", sf::BlendMode::Equation::Subtract},
			{"rs", sf::BlendMode::Equation::ReverseSubtract},
			{"max", sf::BlendMode::Equation::Max},
			{"min", sf::BlendMode::Equation::Min}};

		try
		{
			switch (const auto &bm_args = args.get<std::vector<std::string>>("-bm"); bm_args.size())
			{
			case 0:
				break;

			case 1:
				// use sf::BlendMode static member
				static const std::unordered_map<std::string, sf::BlendMode> default_blendmodes{
					{"alpha", sf::BlendAlpha},
					{"add", sf::BlendAdd},
					{"mult", sf::BlendMultiply},
					{"min", sf::BlendMin},
					{"max", sf::BlendMax},
					{"none", sf::BlendNone}};
				viz.set_spectrum_blendmode(default_blendmodes.at(bm_args[0]));
				break;

			case 3:
				// use first BlendMode constructor
				viz.set_spectrum_blendmode({factor_map.at(bm_args[0]), factor_map.at(bm_args[1]), op_map.at(bm_args[2])});
				break;

			case 6:
				// use second BlendMode constructor
				viz.set_spectrum_blendmode({
					factor_map.at(bm_args[0]),
					factor_map.at(bm_args[1]),
					op_map.at(bm_args[2]),
					factor_map.at(bm_args[3]),
					factor_map.at(bm_args[4]),
					op_map.at(bm_args[5]),
				});
				break;

			default:
				throw std::invalid_argument("--blendmode expects 1, 3, or 6 arguments; see --bm-help");
			}
		}
		catch (const std::exception &e)
		{
			std::cerr << "--blendmode: invalid argument caught; see --bm-help\n";
			_Exit(EXIT_FAILURE);
		}
	}

	// -s, --scale
	switch (const auto &scale_args = args.get<std::vector<std::string>>("-s"); scale_args.size())
	{
	case 0:
		break;
	case 1:
		if (scale_args[0] == "linear")
			viz.set_scale(FS::Scale::LINEAR);
		else if (scale_args[0] == "log")
			viz.set_scale(FS::Scale::LOG);
		else if (scale_args[0] == "nth-root")
			viz.set_scale(FS::Scale::NTH_ROOT);
		break;
	case 2:
		if (scale_args[0] != "nth-root")
			throw std::invalid_argument("only the 'nth-root' scale takes an additional argument");
		viz.set_scale(FS::Scale::NTH_ROOT);
		viz.set_nth_root(std::stoi(scale_args[1]));
		break;
	default:
		throw std::logic_error("-s, --scale: default case hit");
	}
}

void Main::start_in_window(audioviz &viz)
{
#ifdef AUDIOVIZ_PORTAUDIO
	viz.set_audio_playback_enabled(true);
#endif

	sf::RenderWindow window(sf::VideoMode(viz.get_size()), "audioviz", sf::Style::Titlebar, sf::State::Windowed, ctx);
	window.setVerticalSyncEnabled(!no_vsync);

	while (window.isOpen() && viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
			/* resizable windows not happening now, too much of the codebase relies on a static window size
			else if (const auto ev = event.getIf<sf::Event::Resized>())
				viz.set_size(ev->size);
			*/
		}
		window.clear();
	}
}
