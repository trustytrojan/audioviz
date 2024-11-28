#ifdef AUDIOVIZ_LUA

#include "Main.hpp"
#include "base_audioviz.hpp"
#include "media/FfmpegCliBoostMedia.hpp"

sf::IntRect table_to_intrect(const sol::table &tb)
{
	const auto pos = tb[1].get<sol::table>(), size = tb[2].get<sol::table>();
	return {
		pos.empty() ? sf::Vector2i{} : sf::Vector2i{pos[1].get<int>(), pos[2].get<int>()},
		size.empty() ? sf::Vector2i{} : sf::Vector2i{size[1].get<int>(), size[2].get<int>()},
	};
}

sf::Vector2u table_to_vec2u(const sol::table &tb)
{
	if (tb.empty())
		return {};
	return {tb[1].get<uint>(), tb[2].get<uint>()};
}

sf::Color table_to_color(const sol::table &tb)
{
	auto color = sf::Color::Black;
	if (tb[1].valid())
		color.r = tb[1];
	if (tb[2].valid())
		color.g = tb[2];
	if (tb[3].valid())
		color.b = tb[3];
	if (tb[4].valid())
		color.a = tb[4];
	if (tb["r"].valid())
		color.r = tb["r"];
	if (tb["g"].valid())
		color.r = tb["g"];
	if (tb["b"].valid())
		color.b = tb["b"];
	if (tb["a"].valid())
		color.a = tb["a"];
	return color;
}

Main::LuaState::LuaState(Main &main)
{
	open_libraries(sol::lib::base, sol::lib::os); // for testing

	// add more args here!!!!!!!!!
	create_named_table("args", "media_url", main.args.get("media_url"));

	// wrapping arguments with std::ref ensures sol2 will not copy arguments
	set_function("start_in_window", &Main::start_in_window, std::ref(main));
	set("enc_window", &Main::enc_window, std::ref(main));
	set_function("encode", &Main::encode, std::ref(main));

#ifdef LINUX
	set("LINUX", true);
#elifdef _WIN32
	set("WIN32", true);
#endif

	// clang-format off
	auto tt_namespace = create_named_table("tt"),
		 viz_namespace = create_named_table("viz");

	tt_namespace["FrequencyAnalyzer"] = new_usertype<FA>(
		"new", sol::constructors<FA(int)>(),
		"set_fft_size", &FA::set_fft_size,
		"set_interp_type", &FA::set_interp_type,
		"set_window_func", &FA::set_window_func,
		"set_accum_method", &FA::set_accum_method,
		"set_scale", &FA::set_scale,
		"set_nth_root", &FA::set_nth_root
	);

	tt_namespace["AudioAnalyzer"] = new_usertype<tt::AudioAnalyzer>(
		"new", sol::constructors<tt::AudioAnalyzer(int)>(),
		"resize", &tt::AudioAnalyzer::resize
	);

	tt_namespace["StereoAnalyzer"] = new_usertype<tt::StereoAnalyzer>(
		"new", sol::constructors<tt::StereoAnalyzer()>(),
		sol::base_classes, sol::bases<tt::AudioAnalyzer>()
	);

	using SfDrawDrawable = void (sf::RenderTarget::*)(const sf::Drawable &, const sf::RenderStates &);
	using RtClearColor = void (tt::RenderTexture::*)(sf::Color);

	new_usertype<sf::RenderTarget>(
		"", sol::no_constructor,
		"draw",[](sf::RenderTarget &self, const sf::Drawable &drawable)
			{
				self.draw(drawable);
			}
	);

	new_usertype<sf::Sprite>(
		"", sol::no_constructor,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	new_usertype<Media>(
		"", sol::no_constructor
	);

	set("FfmpegCliBoostMedia", new_usertype<FfmpegCliBoostMedia>(
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			return std::make_shared<FfmpegCliBoostMedia>(url, table_to_vec2u(size));
		}),
		sol::base_classes, sol::bases<Media>()
	));

	tt_namespace["RenderTexture"] = new_usertype<tt::RenderTexture>(
		"new", sol::constructors<tt::RenderTexture(sf::Vector2u, int)>(),
		"display", &tt::RenderTexture::display,
		"sprite", &tt::RenderTexture::sprite,
		"clear", [](tt::RenderTexture &self, const sol::table &table)
			{
				self.clear(table_to_color(table));
			},
		sol::base_classes, sol::bases<sf::RenderTarget>()
	);

	viz_namespace["ParticleSystem"] = new_usertype<PS>(
		"new", sol::constructors<PS(int)>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return std::make_shared<PS>(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &PS::set_displacement_direction,
		"set_start_position", &PS::set_start_side,
		"set_rect", &PS::set_rect,
		"set_particle_count", &PS::set_particle_count,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	viz_namespace["SpectrumDrawable"] = new_usertype<SD>(
		"new", sol::constructors<SD(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SD>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SD::set_multiplier,
		"set_rect", &SD::set_rect,
		"set_bar_width", &SD::set_bar_width,
		"set_bar_spacing", &SD::set_bar_spacing,
		"set_backwards", &SD::set_backwards,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	viz_namespace["StereoSpectrum"] = new_usertype<SS>(
		"new", sol::constructors<SS(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SS>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SS::set_multiplier,
		"set_rect", [](SS &self, const sol::table &rect)
		{
			self.set_rect(table_to_intrect(rect));
		},
		"set_bar_width", &SS::set_bar_width,
		"set_bar_spacing", &SS::set_bar_spacing,
		"set_left_backwards", &SS::set_left_backwards,
		"set_right_backwards", &SS::set_right_backwards,
		"update", &SS::update,
		"get_bar_count", &SS::get_bar_count,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	viz_namespace["ScopeDrawable"] = new_usertype<SC>(
		"new", sol::constructors<SC(CS&)>(),
		"new", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SC>(table_to_intrect(rect), cs);
		}),
		"set_rect", &SC::set_rect,
		"set_shape_width", &SC::set_shape_width,
		"set_shape_spacing", &SC::set_shape_spacing,
		"set_fill_in", &SC::set_fill_in,
		"set_backwards", &SC::set_backwards,
		"set_rotation_angle", &SC::set_rotation_angle,
		"set_center_point", &SC::set_center_point,
		sol::base_classes, sol::bases<sf::Drawable>()
	);

	viz_namespace["ColorSettings"] = new_usertype<CS>(
		"new", sol::constructors<CS>(),
		"set_mode", &CS::set_mode,
		"set_wheel_hsv", &CS::set_wheel_hsv,
		"set_wheel_ranges_start_hsv", &CS::set_wheel_ranges_start_hsv,
		"set_wheel_ranges_end_hsv", &CS::set_wheel_ranges_end_hsv,
		"set_wheel_rate", &CS::set_wheel_rate,
		"increment_wheel_time", &CS::increment_wheel_time
	);

	// sf::RenderTexture is non-copyable... (as it should be)
	// had to make the `orig_rt` and `fx_rt` arguments of `viz::Layer::FxCb` MUTABLE references,
	// otherwise sol2 tries to make copies of const-refs...
	viz_namespace["Layer"] = new_usertype<viz::Layer>(
		"", sol::no_constructor,
		"set_orig_cb", &viz::Layer::set_orig_cb,
		"set_fx_cb", &viz::Layer::set_fx_cb,
		"set_auto_fx", &viz::Layer::set_auto_fx,
		"DRAW_FX_RT", sol::var(viz::Layer::DRAW_FX_RT)
	);

	set("base_audioviz", new_usertype<base_audioviz>(
		"new", sol::factories([](const sol::table &rect, Media *const media)
		{
			return std::make_shared<base_audioviz>(table_to_vec2u(rect), media);
		}),
		"add_layer", &base_audioviz::add_layer,
		"get_layer", &base_audioviz::get_layer,
		"remove_layer", &base_audioviz::remove_layer,
		"perform_fft", &base_audioviz::perform_fft,
		"set_text_font", &base_audioviz::set_text_font,
		"set_audio_frames_needed", &base_audioviz::set_audio_frames_needed,
		sol::base_classes, sol::bases<sf::Drawable>()
	));

	set("audioviz", new_usertype<audioviz>(
		"new", sol::factories([](const sol::table &rect, const std::string &media_url, FA &fa, CS &cs, SS &ss, PS &ps, int antialiasing)
		{
			return std::make_shared<audioviz>(table_to_vec2u(rect), media_url, fa, cs, ss, ps, antialiasing);
		}),
		"use_attached_pic_as_bg", &audioviz::use_attached_pic_as_bg,
		"add_default_effects", &audioviz::add_default_effects,
		"set_spectrum_margin", &audioviz::set_spectrum_margin,
		sol::base_classes, sol::bases<base_audioviz>()
	));
	// clang-format on
}

#endif
