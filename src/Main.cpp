#include "Main.hpp"
#include "viz/ParticleSystem.hpp"
#include <unordered_map>

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

	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	audioviz viz{size, args.get("media_url"), fa, ss, ps};
	ps.set_rect({{}, (sf::Vector2i)size});
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
	viz.set_fft_size(args.get<uint>("-n"));

	ss.set_multiplier(args.get<float>("-m"));
	ss.set_bar_width(args.get<uint>("-bw"));
	ss.set_bar_spacing(args.get<uint>("-bs"));

	//add particle default-value changer 
	

	viz.set_framerate(args.get<uint>("-r"));
	no_vsync = args.get<bool>("--no-vsync");
	enc_window = args.get<bool>("--enc-window");

	// no-default-value params
	if (const auto ffpath = args.present("--ffpath"))
		ffmpeg_path = ffpath.value();

	if (const auto bg_path = args.present("--bg"))
		viz.set_background(sf::Texture{*bg_path});
	else
		viz.use_attached_pic_as_bg(); // THIS NEEDS TO BE HERE OTHERWISE THE BACKGROUND BREAKS!!!!!!!!!!!!!

	if (!args.get<bool>("--no-fx"))
		viz.add_default_effects();

	if (const auto album_art_path = args.present("--album-art"))
		viz.set_album_cover(*album_art_path);

	if (const auto font_path = args.present("--font"))
		viz.set_text_font(*font_path);

	{ // accumulation method
		static const std::unordered_map<std::string, FS::AccumulationMethod> am_map{
			{"sum", FS::AccumulationMethod::SUM},
			{"max", FS::AccumulationMethod::MAX},
		};

		const auto &am_str = args.get("-a");

		try
		{
			fa.set_accum_method(am_map.at(am_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--accum-method: unknown accumulation method: " + am_str};
		}
	}

	{ // window function
		static const std::unordered_map<std::string, FS::WindowFunction> wf_map{
			{"hanning", FS::WindowFunction::HANNING},
			{"hamming", FS::WindowFunction::HAMMING},
			{"blackman", FS::WindowFunction::BLACKMAN},
		};

		const auto &wf_str = args.get("-w");

		try
		{
			fa.set_window_func(wf_map.at(wf_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--window-func: unknown window function: " + wf_str};
		}
	}

	{ //start position of particles
		static const std::unordered_map<std::string, viz::ParticleSystem<ParticleShapeType>::StartSide> pos_map{
			{"top", viz::ParticleSystem<ParticleShapeType>::StartSide::TOP},
			{"bottom", viz::ParticleSystem<ParticleShapeType>::StartSide::BOTTOM},
			{"left", viz::ParticleSystem<ParticleShapeType>::StartSide::LEFT},
			{"right", viz::ParticleSystem<ParticleShapeType>::StartSide::RIGHT},
		};

		const auto &pos_str = args.get("-spos");

		try
		{
			ps.set_start_position(pos_map.at(pos_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--start-position: unknown start position: " + pos_str};
		}
	}

	{ // interpolation type
		static const std::unordered_map<std::string, FS::InterpolationType> it_map{
			{"none", FS::InterpolationType::NONE},
			{"linear", FS::InterpolationType::LINEAR},
			{"cspline", FS::InterpolationType::CSPLINE},
			{"cspline_hermite", FS::InterpolationType::CSPLINE_HERMITE}};

		const auto &it_str = args.get("-i");

		try
		{
			fa.set_interp_type(it_map.at(it_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--interp-type: unknown interpolation type: " + it_str};
		}
	}

	{ // spectrum coloring type
		const auto &color_str = args.get("--color");
		if (color_str == "wheel")
		{
			ss.set_color_mode(SD::ColorMode::WHEEL);
			const auto &hsv = args.get<std::vector<float>>("--hsv");
			assert(hsv.size() == 3);
			ss.set_color_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			ss.set_color_wheel_rate(args.get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			ss.set_color_mode(SD::ColorMode::SOLID);
			const auto &rgb = args.get<std::vector<uint8_t>>("--rgb");
			ss.set_solid_color({rgb[0], rgb[1], rgb[2]});
		}
		else
			throw std::invalid_argument{"--color: unknown coloring type: " + color_str};
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
				viz.set_spectrum_blendmode({
					factor_map.at(bm_args[0]),
					factor_map.at(bm_args[1]),
					op_map.at(bm_args[2]),
				});
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
		catch (std::out_of_range)
		{
			throw std::invalid_argument("--blendmode: invalid factor/operation; see --bm-help");
		}
	}

	{ // -s, --scale
		// clang-format off
		static const std::unordered_map<std::string, FS::Scale> scale_map{
			{"linear", FS::Scale::LINEAR},
			{"log", FS::Scale::LOG},
			{"nth-root", FS::Scale::NTH_ROOT}};
		// clang-format on

		const auto &scale_str = args.get("-s");

		try
		{
			fa.set_scale(scale_map.at(scale_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument("--scale: unknown scale: " + scale_str);
		}
	}

	fa.set_nth_root(args.get<int>("--nth-root"));
}

void Main::start_in_window(audioviz &viz)
{
#ifdef AUDIOVIZ_PORTAUDIO
	viz.set_audio_playback_enabled(true);
#endif

	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		"audioviz",
		sf::Style::Titlebar,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	window.setVerticalSyncEnabled(!no_vsync);

	while (window.isOpen() && viz.prepare_frame())
	{
		window.draw(viz);
		window.display();
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
		window.clear();
	}
}
