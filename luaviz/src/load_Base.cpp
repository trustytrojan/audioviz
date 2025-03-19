#include "audioviz/Base.hpp"
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

void table::load_Base()
{
	// clang-format off
	new_usertype<Base>("Base",
		"new", sol::factories([](const sol::table &rect, media::Media *const media)
		{
			return new Base(table_to_vec2<uint>(rect), media);
		}),
		"add_layer", &Base::add_layer,
		"get_layer", &Base::get_layer,
		"remove_layer", &Base::remove_layer,
		"add_final_drawable", &Base::add_final_drawable,
		"perform_fft", &Base::perform_fft,
		"set_text_font", &Base::set_text_font,
		"set_audio_frames_needed", &Base::set_audio_frames_needed,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
