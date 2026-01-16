#include "audioviz/fx/Blur.hpp"
#include <audioviz/Composition.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/media/Media.hpp>
#include <iostream>

constexpr auto fft_size = 3000;
constexpr auto framerate = 60;

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};

	audioviz::FfmpegPopenMedia media{argv[3], 15};

	// audio frames per video frame
	const auto afpvf = media.audio_sample_rate() / framerate;

	// spectrum dependencies
	audioviz::FrequencyAnalyzer fa{fft_size};
	audioviz::AudioAnalyzer aa{media.audio_sample_rate(), fft_size};
	audioviz::BinPacker bp;
	audioviz::Interpolator ip;
	audioviz::ColorSettings color;

	audioviz::SpectrumDrawable spectrum{{{10, 0}, {size.x, size.y - 10}}, color};
	spectrum.set_multiplier(4);

	// create a layer that makes the spectrum drawn to it "glow"
	audioviz::Layer spectrum_layer{"spectrum", size};
	spectrum_layer.add_draw({spectrum});
	spectrum_layer.add_effect(new audioviz::fx::Blur{7.5, 7.5, 15});
	spectrum_layer.set_fx_cb(
		[&](auto &orig_rt, auto &fx_rt, auto &target)
		{
			target.draw(fx_rt.sprite(), audioviz::util::GreatAmazingBlendMode);
			target.draw(orig_rt.sprite());
		});

	sf::RenderWindow window{
		sf::VideoMode{size},
		"spectrum-new-api",
		sf::Style::Titlebar | sf::Style::Close,
		sf::State::Windowed,
	};
	window.setVerticalSyncEnabled(true);

	std::vector<float, aligned_allocator<float>> single_channel_audio(fft_size);

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		const auto audio = media.read_audio(fft_size);
		if (!audio)
		{
			// gracefully close the window
			window.close();
			continue;
		}

		if (media.audio_channels() > 1)
		{
			audioviz::util::extract_channel(single_channel_audio, *audio, media.audio_channels(), 0);
			aa.execute_fft(fa, single_channel_audio);
		}
		else
			aa.execute_fft(fa, *audio);
		spectrum.update(fa, aa, bp, ip);

		// erase the audio we just analyzed, invalidating the AudioChunk's span.
		// the cleaner, but slower alternative to this would be to have AudioChunk's
		// destructor erase its audio from Media's audio buffer.
		media.consume_audio(afpvf);

		window.clear();
		spectrum_layer.full_lifecycle(window);
		window.display();
	}
}
