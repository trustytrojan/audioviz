#include "Main.hpp"

void Main::use_args(ttviz &viz)
{
	// default-value params
	fa.set_fft_size(args.get<uint>("-n"));

	ss.set_multiplier(args.get<float>("-m"));
	ss.set_bar_width(args.get<uint>("-bw"));
	ss.set_bar_spacing(args.get<uint>("-bs"));

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
		static const std::unordered_map<std::string, FA::AccumulationMethod> am_map{
			{"sum", FA::AccumulationMethod::SUM},
			{"max", FA::AccumulationMethod::MAX},
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
		static const std::unordered_map<std::string, FA::WindowFunction> wf_map{
			{"hanning", FA::WindowFunction::HANNING},
			{"hamming", FA::WindowFunction::HAMMING},
			{"blackman", FA::WindowFunction::BLACKMAN},
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

	{ // start position of particles
		static const std::unordered_map<std::string, PS::StartSide> pos_map{
			{"top", PS::StartSide::TOP},
			{"bottom", PS::StartSide::BOTTOM},
			{"left", PS::StartSide::LEFT},
			{"right", PS::StartSide::RIGHT},
		};

		const auto &pos_str = args.get("--ps-startside");

		try
		{
			ps.set_start_side(pos_map.at(pos_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--start-position: unknown start position: " + pos_str};
		}
	}

	{ // interpolation type
		static const std::unordered_map<std::string, FA::InterpolationType> it_map{
			{"none", FA::InterpolationType::NONE},
			{"linear", FA::InterpolationType::LINEAR},
			{"cspline", FA::InterpolationType::CSPLINE},
			{"cspline_hermite", FA::InterpolationType::CSPLINE_HERMITE}};

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
			cs.set_mode(CS::Mode::WHEEL);
			const auto &hsv = args.get<std::vector<float>>("--hsv");
			cs.set_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			cs.set_wheel_rate(args.get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			cs.set_mode(CS::Mode::SOLID);
			const auto &rgb = args.get<std::vector<uint8_t>>("--rgb");
			cs.set_solid_color({rgb[0], rgb[1], rgb[2]});
		}
		else if (color_str.starts_with("wheel_ranges"))
		{
			if (color_str.ends_with("reverse"))
				cs.set_mode(CS::Mode::WHEEL_RANGES_REVERSE);
			else
				cs.set_mode(CS::Mode::WHEEL_RANGES);
			cs.set_wheel_rate(args.get<float>("--wheel-rate"));
			const auto &double_hsv = args.get<std::vector<float>>("--wheel-ranges");
			cs.set_wheel_ranges_start_hsv({double_hsv[0], double_hsv[1], double_hsv[2]});
			cs.set_wheel_ranges_end_hsv({double_hsv[3], double_hsv[4], double_hsv[5]});
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
		static const std::unordered_map<std::string, FA::Scale> scale_map{
			{"linear", FA::Scale::LINEAR},
			{"log", FA::Scale::LOG},
			{"nth-root", FA::Scale::NTH_ROOT}};
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
