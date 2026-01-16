#include "Main.hpp"

Main::Main(const int argc, const char *const *const argv)
	: args{argc, argv}
{
	const auto &size_args = args.get<std::vector<uint>>("--size");
	const sf::Vector2u size{size_args[0], size_args[1]};
	const float start_time = args.get<float>("--start-time");

	audioviz::FfmpegPopenMedia media{args.get("media_url"), size, start_time};
	ttviz viz{size, media, args.get<uint>("-n")};

	configure_from_args(viz);

	// --encode: render to video file
	switch (const auto &encode_args = args.get<std::vector<std::string>>("--encode"); encode_args.size())
	{
	case 0:
		viz.start_in_window(media, "ttviz");
		break;
	case 1:
		viz.encode(media, encode_args[0]);
		break;
	case 2:
		viz.encode(media, encode_args[0], encode_args[1]);
		break;
	case 3:
		viz.encode(media, encode_args[0], encode_args[1], encode_args[2]);
		break;
	default:
		throw std::logic_error{"--encode requires 1-3 arguments"};
	}
}
void Main::configure_from_args(ttviz &viz)
{
	auto &fa = viz.get_fa();
	auto &bp = viz.get_bp();
	auto &ip = viz.get_ip();
	auto &color = viz.get_color();
	auto &ss = viz.get_ss();
	auto &ps = viz.get_ps();

	// Basic settings
	ss.set_bar_width(args.get<uint>("-bw"));
	ss.set_bar_spacing(args.get<uint>("-bs"));

	const float multiplier = args.get<float>("-m");
	ss.set_multiplier(multiplier);

	viz.set_framerate(args.get<uint>("-r"));
	viz.set_timing_text_enabled(args.get<bool>("--timing-text"));
	ps.set_framerate(viz.get_framerate());

	// Optional settings
	if (const auto bg_path = args.present("--bg"))
		viz.set_background(sf::Texture{*bg_path});

	if (!args.get<bool>("--no-fx"))
		viz.add_default_effects();

	if (const auto album_art_path = args.present("--album-art"))
		viz.set_album_cover(*album_art_path);

	if (const auto font_path = args.present("--font"))
		viz.set_profiler_font(*font_path);

	// Accumulation method
	{
		static const std::unordered_map<std::string, audioviz::BinPacker::AccumulationMethod> am_map{
			{"sum", audioviz::BinPacker::AccumulationMethod::SUM},
			{"max", audioviz::BinPacker::AccumulationMethod::MAX},
		};

		const auto &am_str = args.get("-a");
		try
		{
			bp.set_accum_method(am_map.at(am_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--accum-method: unknown accumulation method: " + am_str};
		}
	}

	// Window function
	{
		static const std::unordered_map<std::string, audioviz::FrequencyAnalyzer::WindowFunction> wf_map{
			{"none", audioviz::FrequencyAnalyzer::WindowFunction::None},
			{"hanning", audioviz::FrequencyAnalyzer::WindowFunction::Hanning},
			{"hamming", audioviz::FrequencyAnalyzer::WindowFunction::Hamming},
			{"blackman", audioviz::FrequencyAnalyzer::WindowFunction::Blackman},
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

	// Interpolation type
	{
		static const std::unordered_map<std::string, audioviz::Interpolator::InterpolationType> it_map{
			{"linear", audioviz::Interpolator::InterpolationType::LINEAR},
			{"cspline", audioviz::Interpolator::InterpolationType::CSPLINE},
			{"cspline_hermite", audioviz::Interpolator::InterpolationType::CSPLINE_HERMITE}};

		const auto &it_str = args.get("-i");
		try
		{
			ip.set_interp_type(it_map.at(it_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument{"--interp-type: unknown interpolation type: " + it_str};
		}
	}

	// Spectrum coloring type
	{
		const auto &color_str = args.get("--color");
		if (color_str == "wheel")
		{
			color.set_mode(audioviz::ColorSettings::Mode::WHEEL);
			const auto &hsv = args.get<std::vector<float>>("--hsv");
			color.set_wheel_hsv({hsv[0], hsv[1], hsv[2]});
			color.set_wheel_rate(args.get<float>("--wheel-rate"));
		}
		else if (color_str == "solid")
		{
			color.set_mode(audioviz::ColorSettings::Mode::SOLID);
			const auto &rgb = args.get<std::vector<uint8_t>>("--rgb");
			color.set_solid_color({rgb[0], rgb[1], rgb[2]});
		}
		else if (color_str.starts_with("wheel_ranges"))
		{
			if (color_str.ends_with("reverse"))
				color.set_mode(audioviz::ColorSettings::Mode::WHEEL_RANGES_REVERSE);
			else
				color.set_mode(audioviz::ColorSettings::Mode::WHEEL_RANGES);
			color.set_wheel_rate(args.get<float>("--wheel-rate"));
			const auto &double_hsv = args.get<std::vector<float>>("--wheel-ranges");
			color.set_wheel_ranges_start_hsv({double_hsv[0], double_hsv[1], double_hsv[2]});
			color.set_wheel_ranges_end_hsv({double_hsv[3], double_hsv[4], double_hsv[5]});
		}
		else
			throw std::invalid_argument{"--color: unknown coloring type: " + color_str};
	}

	// Spectrum blendmode
	{
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
			{
				static const std::unordered_map<std::string, sf::BlendMode> default_blendmodes{
					{"alpha", sf::BlendAlpha},
					{"add", sf::BlendAdd},
					{"mult", sf::BlendMultiply},
					{"min", sf::BlendMin},
					{"max", sf::BlendMax},
					{"none", sf::BlendNone}};
				viz.set_spectrum_blendmode(default_blendmodes.at(bm_args[0]));
				break;
			}

			case 3:
				viz.set_spectrum_blendmode({
					factor_map.at(bm_args[0]),
					factor_map.at(bm_args[1]),
					op_map.at(bm_args[2]),
				});
				break;

			case 6:
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

	// Scale
	{
		static const std::unordered_map<std::string, audioviz::BinPacker::Scale> scale_map{
			{"linear", audioviz::BinPacker::Scale::LINEAR},
			{"log", audioviz::BinPacker::Scale::LOG},
			{"nth-root", audioviz::BinPacker::Scale::NTH_ROOT}};

		const auto &scale_str = args.get("-s");
		try
		{
			bp.set_scale(scale_map.at(scale_str));
		}
		catch (std::out_of_range)
		{
			throw std::invalid_argument("--scale: unknown scale: " + scale_str);
		}
	}

	bp.set_nth_root(args.get<int>("--nth-root"));
}
