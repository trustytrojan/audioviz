#include "media/Media.hpp"
#include "media/FfmpegCliBoostMedia.hpp"
#include "tt/FrequencyAnalyzer.hpp"
#include "viz/ColorSettings.hpp"
#include "viz/ScopeDrawable.hpp"
#include "viz/SpectrumDrawable.hpp"
#include "viz/VerticalBar.hpp"
#include <iostream>
#include <portaudio.hpp>

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{atoi(argv[1]), atoi(argv[2])};
	sf::RenderWindow window{sf::VideoMode{size}, "ScopeDrawableTest"};
	window.setVerticalSyncEnabled(true);

	viz::ColorSettings color;
	color.wheel.rate = .005;
	color.mode = viz::ColorSettings::Mode::WHEEL_RANGES;

	sf::IntRect rect({}, (sf::Vector2i)size);

	viz::ScopeDrawable<sf::RectangleShape> scope(rect, color);
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);
	scope.set_fill_in(true);

	viz::SpectrumDrawable<viz::VerticalBar> sd(rect, color);
	sd.set_bar_width(1);
	sd.set_bar_spacing(0);

	const auto fft_size = size.x;
	tt::FrequencyAnalyzer fa{fft_size};

	std::unique_ptr<Media> media{new FfmpegCliBoostMedia{argv[3]}};
	// media->init(size);

	int afpvf{media->astream().sample_rate() / 60};

	std::vector<float> left_channel(size.x), spectrum(fft_size);

	pa::PortAudio _;
	pa::Stream pa_stream{0, media->astream().nb_channels(), paFloat32, media->astream().sample_rate()};
	pa_stream.start();

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		{
			media->decode_audio(size.x);

			if (media->audio_buffer().size() < size.x)
				break;

			// copy just the left channel
			for (int i = 0; i < size.x; ++i)
				left_channel[i] = media->audio_buffer()[i * media->astream().nb_channels() + 0 /* left channel */];
			scope.update(left_channel);
			fa.copy_to_input(left_channel.data());
			fa.render(spectrum);
			sd.update(spectrum);
			color.wheel.increment_time();
			// sd.color_wheel_increment();
			// scope.color_wheel_increment();

			try
			{
				pa_stream.write(media->audio_buffer().data(), afpvf);
			}
			catch (const pa::Error &e)
			{
				if (e.code != paOutputUnderflowed)
					throw;
				std::cerr << e.what() << '\n';
			}
			media->audio_buffer_erase(afpvf);
		}

		window.clear();
		window.draw(scope);
		window.draw(sd);
		window.display();
	}
}
