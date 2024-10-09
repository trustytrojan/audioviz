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
	open_libraries(sol::lib::base); // for testing

	set_function("start_in_window", &Main::start_in_window, main);
	set_function("encode", &Main::encode, main);

	// clang-format off
	auto tt_namespace = create_named_table("tt"),
		 viz_namespace = create_named_table("viz");

	tt_namespace["FrequencyAnalyzer"] = new_usertype<tt::FrequencyAnalyzer>(
		"", sol::constructors<tt::FrequencyAnalyzer(int)>(),
		"set_fft_size", &tt::FrequencyAnalyzer::set_fft_size,
		"set_interp_type", &tt::FrequencyAnalyzer::set_interp_type,
		"set_window_func", &tt::FrequencyAnalyzer::set_window_func,
		"set_accum_method", &tt::FrequencyAnalyzer::set_accum_method,
		"set_scale", &tt::FrequencyAnalyzer::set_scale,
		"set_nth_root", &tt::FrequencyAnalyzer::set_nth_root
	);

	viz_namespace["ParticleSystem"] = new_usertype<viz::ParticleSystem<ParticleShapeType>>(
		"", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return std::make_shared<viz::ParticleSystem<ParticleShapeType>>(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &viz::ParticleSystem<ParticleShapeType>::set_displacement_direction,
		"set_start_position", &viz::ParticleSystem<ParticleShapeType>::set_start_position,
		"set_rect", &viz::ParticleSystem<ParticleShapeType>::set_rect,
		"set_particle_count", &viz::ParticleSystem<ParticleShapeType>::set_particle_count
	);

	viz_namespace["SpectrumDrawable"] = new_usertype<viz::SpectrumDrawable<BarType>>(
		"", sol::constructors<viz::SpectrumDrawable<BarType>>(),
		"set_multiplier", &viz::SpectrumDrawable<BarType>::set_multiplier,
		"set_rect", &viz::SpectrumDrawable<BarType>::set_rect,
		"set_bar_width", &viz::SpectrumDrawable<BarType>::set_bar_width,
		"set_bar_spacing", &viz::SpectrumDrawable<BarType>::set_bar_spacing,
		"set_color_mode", &viz::SpectrumDrawable<BarType>::set_color_mode,
		"set_color_wheel_hsv", &viz::SpectrumDrawable<BarType>::set_color_wheel_hsv,
		"set_color_wheel_rate", &viz::SpectrumDrawable<BarType>::set_color_wheel_rate,
		"set_solid_color", &viz::SpectrumDrawable<BarType>::set_solid_color,
		"set_backwards", &viz::SpectrumDrawable<BarType>::set_backwards
	);

	viz_namespace["StereoSpectrum"] = new_usertype<viz::StereoSpectrum<BarType>>(
		"", sol::constructors<viz::StereoSpectrum<BarType>>(),
		"set_multiplier", &viz::StereoSpectrum<BarType>::set_multiplier,
		"set_rect", &viz::StereoSpectrum<BarType>::set_rect,
		"set_bar_width", &viz::StereoSpectrum<BarType>::set_bar_width,
		"set_bar_spacing", &viz::StereoSpectrum<BarType>::set_bar_spacing,
		"set_color_mode", &viz::StereoSpectrum<BarType>::set_color_mode,
		"set_color_wheel_hsv", &viz::StereoSpectrum<BarType>::set_color_wheel_hsv,
		"set_color_wheel_rate", &viz::StereoSpectrum<BarType>::set_color_wheel_rate,
		"set_solid_color", &viz::StereoSpectrum<BarType>::set_solid_color,
		"set_left_backwards", &viz::StereoSpectrum<BarType>::set_left_backwards,
		"set_right_backwards", &viz::StereoSpectrum<BarType>::set_right_backwards
	);

	new_usertype<audioviz>("audioviz",
		"new", sol::factories([](
			const sol::table &rect,
			const std::string &media_url,
			tt::FrequencyAnalyzer &fa,
			viz::StereoSpectrum<BarType> &ss,
			viz::ParticleSystem<ParticleShapeType> &ps,
			int antialiasing)
		{
			return std::make_shared<audioviz>(table_to_vec2u(rect), media_url, fa, ss, ps, antialiasing);
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
		"set_media_url", &audioviz::set_media_url,
		"set_fft_size", &audioviz::set_fft_size
	);
	// clang-format on
}

#endif
