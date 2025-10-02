#include <audioviz/Base.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/media/Media.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <portaudio.hpp>

struct ScopeTest : audioviz::Base
{
	audioviz::ColorSettings colorL, colorR;
	audioviz::ScopeDrawable<sf::CircleShape> scopeL, scopeR;
	ScopeTest(sf::Vector2u size, const std::string &media_url);
};

ScopeTest::ScopeTest(sf::Vector2u size, const std::string &media_url)
	: Base{size, new audioviz::FfmpegPopenMedia{media_url, size}},
	  scopeL{{{}, (sf::Vector2i)size}, colorL},
	  scopeR{{{}, (sf::Vector2i)size}, colorR}
{
	colorL.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorL.set_solid_color(sf::Color::Red);
	colorR.set_mode(audioviz::ColorSettings::Mode::SOLID);
	colorR.set_solid_color(sf::Color::Cyan);

	const auto width = 10, spacing = 3;

	scopeL.set_shape_spacing(spacing);
	scopeR.set_shape_spacing(spacing);
	scopeL.set_shape_width(width);
	scopeR.set_shape_width(width);
	assert(scopeL.get_shape_count() == scopeR.get_shape_count());
	scopeL.set_fill_in(true);
	scopeR.set_fill_in(true);

	set_audio_playback_enabled(true);

	auto &scope_layer = add_layer("scope");
	scope_layer.add_drawable(&scopeL);
	scope_layer.add_drawable(&scopeR);
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
			scopeL.update({left_channel, afpvf});
			scopeR.update({right_channel, afpvf});
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

	const sf::Vector2u size{atoi(argv[1]), atoi(argv[2])};
	ScopeTest viz{size, argv[3]};
}
