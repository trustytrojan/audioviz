#include "table.hpp"
#include <audioviz/media/FfmpegPopenMedia.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_media()
{
	// clang-format off
	new_usertype<Media>("Media",
		"", sol::no_constructor,
		"attached_pic", [](const Media& self) -> sol::optional<sf::Texture>
		{
			if (const auto pic = self.attached_pic())
				return *pic;
			return sol::nullopt;
		},
		"audio_sample_rate", &Media::audio_sample_rate
	);

	new_usertype<FfmpegPopenMedia>("FfmpegPopenMedia",
		sol::base_classes, sol::bases<Media>(),
		"new", sol::constructors<FfmpegPopenMedia(const std::string &, float)>()
	);
	// clang-format on
}

} // namespace luaviz
