#include "Main.hpp"
#include <sol.hpp>

sol::state Main::lua_init()
{
    sol::state lua;
	// lua.open_libraries(sol::lib::base);
    lua.new_usertype<audioviz>("audioviz",
		// custom factory to convert { width, height } to sf::Vector2u
        "new", sol::factories([](const sol::table args, const std::string &media_url)
		{
			const unsigned int width = args[1],
							   height = args[2];
			const sf::Vector2u size(width, height);
			return std::make_shared<audioviz>(size, media_url);
		}),
		
		// setters
#ifdef PORTAUDIO
        "set_audio_playback_enabled", &audioviz::set_audio_playback_enabled,
#endif
        "set_framerate", &audioviz::set_framerate,
        "set_background", static_cast<void(audioviz::*)(const std::string &)>(&audioviz::set_background),
        "set_spectrum_margin", &audioviz::set_spectrum_margin,
        "set_spectrum_blendmode", &audioviz::set_spectrum_blendmode,
        "set_album_cover", &audioviz::set_album_cover,
        "set_text_font", &audioviz::set_text_font,
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

		// layer objects
		"bg", &audioviz::bg,
		"spectrum", &audioviz::spectrum,
		"particles", &audioviz::particles
    );
    return lua;
}