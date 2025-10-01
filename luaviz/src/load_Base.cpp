#include "audioviz/Base.hpp"
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

void table::load_Base()
{
	// clang-format off
	new_usertype<Base>("Base",
		"new", sol::factories([](const sol::table &rect, Media *const media)
		{
			// the `uint` typedef isnt on mingw
			return new Base(table_to_vec2<unsigned>(rect), media);
		}),
		"add_layer", &Base::add_layer,
		"get_layer", &Base::get_layer,
		"remove_layer", &Base::remove_layer,
		"add_final_drawable", &Base::add_final_drawable,
		"perform_fft", &Base::perform_fft,
		"set_text_font", &Base::set_text_font,
		"set_audio_frames_needed", &Base::set_audio_frames_needed,
		"start_in_window", &Base::start_in_window,
		"encode", &Base::encode,
		"get_framerate", &Base::get_framerate,
		"set_timing_text_enabled", &Base::set_timing_text_enabled,
		"set_audio_playback_enabled", &Base::set_audio_playback_enabled,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
