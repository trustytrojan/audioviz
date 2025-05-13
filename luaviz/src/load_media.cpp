#include "table.hpp"
#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include <iostream>

using namespace audioviz;

namespace luaviz
{

void table::load_media()
{
	// clang-format off
	new_usertype<media::Media>("Media",
		"", sol::no_constructor,
		"attached_pic", [](const media::Media& self) -> sol::optional<sf::Texture> {
			if (const auto pic = self.attached_pic())
				return *pic;
			return sol::nullopt;
		}
	);

	new_usertype<media::FfmpegCliBoostMedia>("FfmpegCliBoostMedia",
		sol::base_classes, sol::bases<media::Media>(),
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			auto m = new media::FfmpegCliBoostMedia(url, table_to_vec2<uint>(size));
			std::cerr << "luaviz: FfmpegCliBoostMedia: m=" << m << '\n';
			return m;
		})
	);
	// clang-format on
}

} // namespace luaviz
