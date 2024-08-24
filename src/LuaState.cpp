#include "Main.hpp"

Main::LuaState::LuaState(Main &main)
{
	open_libraries(sol::lib::base); // for testing

	set_function("start_in_window", &Main::start_in_window, main);
	set_function("encode", &Main::encode_without_window, main);

	// clang-format off
    new_usertype<audioviz>("audioviz",
		// custom factory to convert { width, height } to sf::Vector2u
        "new", sol::factories([](const sol::table args, const std::string &media_url)
		{
			const unsigned int width = args[1],
							   height = args[2];
			const sf::Vector2u size(width, height);
			return std::make_shared<audioviz>(size, media_url);
		}),
		
		// setters/customization
		"set_timing_text_enabled", &audioviz::set_timing_text_enabled,
		"set_media_url", &audioviz::set_media_url,
		"add_default_effects", &audioviz::add_default_effects,
        "set_framerate", &audioviz::set_framerate,
        "set_background", static_cast<void(audioviz::*)(const std::string &)>(&audioviz::set_background),
        "set_spectrum_margin", &audioviz::set_spectrum_margin,
        "set_spectrum_blendmode", &audioviz::set_spectrum_blendmode,
        "set_album_cover", &audioviz::set_album_cover,
        "set_text_font", &audioviz::set_text_font,
#ifdef PORTAUDIO
        "set_audio_playback_enabled", &audioviz::set_audio_playback_enabled,
#endif

		// passthrough setters
        "set_sample_size", &audioviz::set_sample_size,
        "set_bar_width", &audioviz::set_bar_width,
        "set_bar_spacing", &audioviz::set_bar_spacing,
        "set_color_mode", &audioviz::set_color_mode,
        "set_solid_color", &audioviz::set_solid_color,
        "set_color_wheel_rate", &audioviz::set_color_wheel_rate,
        "set_color_wheel_hsv", &audioviz::set_color_wheel_hsv,
        "set_multiplier", &audioviz::set_multiplier,
        "set_fft_size", &audioviz::set_fft_size,
        "set_interp_type", &audioviz::set_interp_type,
        "set_scale", &audioviz::set_scale,
        "set_nth_root", &audioviz::set_nth_root,
        "set_accum_method", &audioviz::set_accum_method,
        "set_window_func", &audioviz::set_window_func,

		// layers (no idea how to make this work)
		"bg", &audioviz::bg,
		"spectrum", &audioviz::spectrum,
		"particles", &audioviz::particles
    );
	// clang-format on
}