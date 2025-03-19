#include <audioviz/fft/StereoAnalyzer.hpp>
#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include <audioviz/util.hpp>
#include <audioviz/ColorSettings.hpp>
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

sf::IntRect table_to_intrect(const sol::table &tb)
{
	const auto pos = tb[1].get<sol::table>(), size = tb[2].get<sol::table>();
	return sf::IntRect{{pos.get_or(1, 0), pos.get_or(2, 0)}, {size.get_or(1, 0), size.get_or(2, 0)}};
}

sf::Color table_to_color(const sol::table &tb)
{
	auto color = sf::Color::Black;
	if (tb[1].valid())
		color.r = tb[1];
	if (tb[2].valid())
		color.g = tb[2];
	if (tb[3].valid())
		color.b = tb[3];
	if (tb[4].valid())
		color.a = tb[4];
	if (tb["r"].valid())
		color.r = tb["r"];
	if (tb["g"].valid())
		color.r = tb["g"];
	if (tb["b"].valid())
		color.b = tb["b"];
	if (tb["a"].valid())
		color.a = tb["a"];
	return color;
}

table::table(const sol::table &t)
	: sol::table{t}
{
	using FA = fft::FrequencyAnalyzer;
	using AA = fft::AudioAnalyzer;
	using SA = fft::StereoAnalyzer;
	using CS = ColorSettings;

	// clang-format off

	load_FA();

	new_usertype<AA>("AudioAnalyzer",
		"new", sol::constructors<AA(int)>(),
		"resize", &AA::resize
	);

	new_usertype<SA>("StereoAnalyzer",
		"new", sol::constructors<SA()>(),
		sol::base_classes, sol::bases<AA>()
	);

	load_RT();
	load_Layer();
	load_CPS();

	new_enum("ColorMode",
		"SOLID", CS::Mode::SOLID,
		"WHEEL", CS::Mode::WHEEL,
		"WHEEL_RANGES", CS::Mode::WHEEL_RANGES,
		"WHEEL_RANGES_REVERSE", CS::Mode::WHEEL_RANGES_REVERSE
	);

	load_CS();
	load_BSD();
	load_BSS();
	load_BSC();

	new_usertype<media::Media>("Media",
		"", sol::no_constructor,
		"attached_pic", [](const media::Media& self) -> sol::optional<sf::Texture> {
			if (const auto pic = self.attached_pic())
				return *pic;
			return sol::nullopt;
		}
	);

	new_usertype<media::FfmpegCliBoostMedia>("FfmpegCliBoostMedia",
		"new", sol::factories([](const std::string &url, const sol::table &size)
		{
			return new media::FfmpegCliBoostMedia(url, table_to_vec2<uint>(size));
		}),
		sol::base_classes, sol::bases<media::Media>()
	);

	load_sf_types();
	load_Sprite();
	load_SMD();
	load_Base();

	set_function("start_in_window", &util::start_in_window);

	// clang-format on
}

} // namespace luaviz
