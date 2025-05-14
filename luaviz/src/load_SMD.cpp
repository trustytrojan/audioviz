#include "audioviz/SongMetadataDrawable.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_SMD()
{
	using SMD = SongMetadataDrawable;
	// clang-format off
	new_enum("SMDTextPosition",
		"RIGHT", SMD::TextPosition::RIGHT,
		"BOTTOM", SMD::TextPosition::BOTTOM
	);

	new_usertype<SMD>("SongMetadataDrawable",
		"new", sol::constructors<SMD(sf::Text &, sf::Text &)>(),
		"use_metadata", &SMD::use_metadata,
		"set_album_cover", [](SMD &self, const sf::Texture &txr, const sol::table &size)
		{
			self.set_album_cover(txr, table_to_vec2<float>(size));
		},
		"set_position", [](SMD &self, const sol::table &pos)
		{
			self.set_position(table_to_vec2<float>(pos));
		},
		"set_text_pos", &SMD::set_text_pos,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
