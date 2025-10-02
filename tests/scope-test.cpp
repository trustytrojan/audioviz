#include <audioviz/Base.hpp>
#include <audioviz/StereoScope.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <portaudio.hpp>

struct ScopeTest : audioviz::Base
{
	audioviz::ColorSettings colorL, colorR;
	audioviz::StereoScope<sf::RectangleShape> stereo_scope;
	ScopeTest(sf::Vector2u size, const std::string &media_url);
};

ScopeTest::ScopeTest(sf::Vector2u size, const std::string &media_url)
	: Base{size, new audioviz::FfmpegPopenMedia{media_url, size}},
	  stereo_scope{{{}, (sf::Vector2i)size}, colorL, colorR}
{
	colorL.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorL.set_solid_color(sf::Color::Red);
	colorR.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorR.set_solid_color(sf::Color::Cyan);

	stereo_scope.set_shape_width(1);
	stereo_scope.set_shape_spacing(2);
	// stereo_scope.set_fill_in(true);

	set_audio_playback_enabled(true);

	auto &scope_layer = add_layer("scope");
	scope_layer.add_drawable(&stereo_scope);
	scope_layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			float left_channel[afpvf];
			float right_channel[afpvf];
			for (int i = 0; i < afpvf; ++i)
			{
				const auto &buf = media->audio_buffer();
				const auto frame_idx = i * media->audio_channels();
				left_channel[i] = buf[frame_idx];
				right_channel[i] = buf[frame_idx + 1];
			}
			stereo_scope.update({left_channel, afpvf}, {right_channel, afpvf});
		});

	start_in_window("scope-test");
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	ScopeTest viz{{atoi(argv[1]), atoi(argv[2])}, argv[3]};
}
