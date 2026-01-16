#include <audioviz/Base.hpp>
#include <audioviz/StereoScope.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

struct ScopeTest : audioviz::Base
{
	audioviz::ColorSettings colorL, colorR;
	audioviz::StereoScope<sf::RectangleShape> stereo_scope;
	ScopeTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

ScopeTest::ScopeTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  stereo_scope{{{}, (sf::Vector2i)size}, colorL, colorR}
{
	colorL.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorL.set_solid_color(sf::Color::Red);
	colorR.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorR.set_solid_color(sf::Color::Cyan);

	stereo_scope.set_shape_width(10);
	stereo_scope.set_shape_spacing(5);
	// stereo_scope.set_fill_in(true);

#ifdef __linux__
	enable_profiler();
	set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	// if we make this a layer, we can capture the full draw time
	add_final_drawable(stereo_scope);

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window(media, "scope-test");
}

void ScopeTest::update(const std::span<const float> audio_buffer)
{
	float left_channel[afpvf], right_channel[afpvf];
	capture_time(
		"audio_split", for (int i = 0; i < afpvf; ++i) {
			const auto frame_idx = i * 2; // assuming stereo
			left_channel[i] = audio_buffer[frame_idx];
			right_channel[i] = audio_buffer[frame_idx + 1];
		});
	capture_time("sc_update", stereo_scope.update({left_channel, afpvf}, {right_channel, afpvf}));
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	ScopeTest viz{size, argv[3]};
}
