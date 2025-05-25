#include <audioviz/ColorSettings.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/media/Media.hpp>
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

	audioviz::ColorSettings color;
	color.set_wheel_rate(.005);
	color.set_mode(audioviz::ColorSettings::Mode::WHEEL_RANGES);

	sf::IntRect rect({}, (sf::Vector2i)size);

	audioviz::ScopeDrawable<sf::RectangleShape> scope(rect, color);
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);
	scope.set_fill_in(true);

	audioviz::SpectrumDrawable<audioviz::VerticalBar> sd(rect, color);
	sd.set_bar_width(1);
	sd.set_bar_spacing(0);

	const auto fft_size = size.x;
	audioviz::fft::FrequencyAnalyzer fa{fft_size};

	std::unique_ptr<audioviz::Media> media{audioviz::Media::create(argv[3])};

	int afpvf{media->audio_sample_rate() / 60};

	std::vector<float> left_channel(size.x), spectrum(fft_size);

	pa::PortAudio _;
	pa::Stream pa_stream{0, media->audio_channels(), paFloat32, media->audio_sample_rate()};
	pa_stream.start();

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		{
			media->buffer_audio(size.x);

			if (media->audio_buffer().size() < size.x)
				break;

			// copy just the left channel
			for (int i = 0; i < size.x; ++i)
				left_channel[i] = media->audio_buffer()[i * media->audio_channels() + 0 /* left channel */];
			scope.update(left_channel);
			fa.copy_to_input(left_channel.data());
			fa.render(spectrum);
			sd.update(spectrum);
			color.increment_wheel_time();

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
