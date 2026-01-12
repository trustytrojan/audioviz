#include "Base.hpp"
#include "table.hpp"

namespace luaviz
{

void table::load_Base()
{
	// clang-format off
	new_usertype<std::span<const float>>("const_float_span",
		"", sol::no_constructor,
		"data", &std::span<const float>::data
	);

	new_usertype<Base>("Base",
		"new", sol::factories([](const sol::table &rect, audioviz::Media &media)
		{
			// the `uint` typedef isnt on mingw
			return new Base(table_to_vec2<unsigned>(rect), media);
		}),
		"add_layer", &Base::add_layer,
		"get_layer", &Base::get_layer,
		"remove_layer", &Base::remove_layer,
		"add_final_drawable", &Base::add_final_drawable,
		"add_final_drawable2", &Base::add_final_drawable2,
		"set_text_font", &Base::set_text_font,
		"set_audio_frames_needed", &Base::set_audio_frames_needed,
		"start_in_window", &Base::start_in_window,
		"encode", &Base::encode,
		"get_framerate", &Base::get_framerate,
		"set_framerate", &Base::set_framerate,
		"set_timing_text_enabled", &Base::set_timing_text_enabled,
		"perform_fft", &Base::perform_fft,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
