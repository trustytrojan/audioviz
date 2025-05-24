#include "table.hpp"
#ifdef AUDIOVIZ_BOOST
#include <audioviz/media/FfmpegBoostMedia.hpp>
#endif
#include <audioviz/media/FfmpegPopenMedia.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_media()
{
	// clang-format off
	new_usertype<media::Media>("Media",
		"", sol::no_constructor,
		"attached_pic", [](const media::Media& self) -> sol::optional<sf::Texture>
		{
			if (const auto pic = self.attached_pic())
				return *pic;
			return sol::nullopt;
		}
	);

#ifdef AUDIOVIZ_BOOST
	new_usertype<media::FfmpegCliBoostMedia>("FfmpegBoostMedia",
		sol::base_classes, sol::bases<media::Media>(),
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			return new media::FfmpegCliBoostMedia{url, table_to_vec2<unsigned>(size)};
		})
	);
#endif

	new_usertype<media::FfmpegPopenMedia>("FfmpegPopenMedia",
		sol::base_classes, sol::bases<media::Media>(),
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			return new media::FfmpegPopenMedia{url, table_to_vec2<unsigned>(size)};
		})
	);
	// clang-format on
}

} // namespace luaviz
