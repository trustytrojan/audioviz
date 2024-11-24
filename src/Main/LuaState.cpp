#ifdef AUDIOVIZ_LUA

#include "Main.hpp"

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

Main::LuaState::LuaState(Main &main)
{
	open_libraries(sol::lib::base, sol::lib::os); // for testing

	// add more args here!!!!!!!!!
	create_named_table("args",
		"media_url", main.args.get("media_url")
	);

	// wrapping arguments with std::ref ensures sol2 will not copy arguments
	set_function("start_in_window", &Main::start_in_window, std::ref(main));
	set_function("encode_without_window", &Main::encode_without_window, std::ref(main));
	set_function("encode_without_window_mt", &Main::encode_without_window_mt, std::ref(main));
	set_function("encode_with_window", &Main::encode_with_window, std::ref(main));

	// clang-format off
	auto tt_namespace = create_named_table("tt"),
		 viz_namespace = create_named_table("viz"),
		 sf_namespace = create_named_table("sf");

	tt_namespace["FrequencyAnalyzer"] = new_usertype<FA>(
		"", sol::constructors<FA(int)>(),
		"set_fft_size", &FA::set_fft_size,
		"set_interp_type", &FA::set_interp_type,
		"set_window_func", &FA::set_window_func,
		"set_accum_method", &FA::set_accum_method,
		"set_scale", &FA::set_scale,
		"set_nth_root", &FA::set_nth_root
	);

	viz_namespace["ParticleSystem"] = new_usertype<PS>(
		"", sol::constructors<PS(int)>(),
		"", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return std::make_shared<PS>(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &PS::set_displacement_direction,
		"set_start_position", &PS::set_start_side,
		"set_rect", &PS::set_rect,
		"set_particle_count", &PS::set_particle_count
	);

	viz_namespace["SpectrumDrawable"] = new_usertype<SD>(
		"", sol::constructors<SD(CS&)>(),
		"", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SD>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SD::set_multiplier,
		"set_rect", &SD::set_rect,
		"set_bar_width", &SD::set_bar_width,
		"set_bar_spacing", &SD::set_bar_spacing,
		"set_backwards", &SD::set_backwards
	);

	viz_namespace["StereoSpectrum"] = new_usertype<SS>(
		"", sol::constructors<SS(CS&)>(),
		"", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SS>(table_to_intrect(rect), cs);
		}),
		"set_multiplier", &SS::set_multiplier,
		"set_rect", &SS::set_rect,
		"set_bar_width", &SS::set_bar_width,
		"set_bar_spacing", &SS::set_bar_spacing,
		"set_left_backwards", &SS::set_left_backwards,
		"set_right_backwards", &SS::set_right_backwards
	);

	viz_namespace["ScopeDrawable"] = new_usertype<SC>(
		"", sol::constructors<SC(CS&)>(),
		"", sol::factories([](const sol::table &rect, CS &cs)
		{
			return std::make_shared<SC>(table_to_intrect(rect), cs);
		}),
		"set_rect", &SC::set_rect,
		"set_shape_width", &SC::set_shape_width,
		"set_shape_spacing", &SC::set_shape_spacing,
		"set_fill_in", &SC::set_fill_in,
		"set_backwards", &SC::set_backwards,
		"set_rotation_angle", &SC::set_rotation_angle,
		"set_center_point", &SC::set_center_point
	);

	viz_namespace["ColorSettings"] = new_usertype<CS>(
		"", sol::constructors<CS>()
	);

	new_usertype<audioviz>("audioviz",
		"new", sol::factories([](const sol::table &rect, const std::string &media_url, FA &fa, CS &cs, SS &ss, PS &ps, int antialiasing)
		{
			return std::make_shared<audioviz>(table_to_vec2u(rect), media_url, fa, cs, ss, ps, antialiasing);
		}),
		"use_attached_pic_as_bg", &audioviz::use_attached_pic_as_bg,
		"add_default_effects", &audioviz::add_default_effects,
		"add_layer", &audioviz::add_layer,
		"get_layer", &audioviz::get_layer,
#ifdef AUDIOVIZ_PORTAUDIO
		"set_audio_playback_enabled", &audioviz::set_audio_playback_enabled,
#endif
		"set_timing_text_enabled", &audioviz::set_timing_text_enabled,
		"set_framerate", &audioviz::set_framerate,
		"set_spectrum_margin", &audioviz::set_spectrum_margin,
		"set_text_font", &audioviz::set_text_font,
		"set_fft_size", &audioviz::set_fft_size
	);
	// clang-format on
}

#endif
