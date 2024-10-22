#include "viz/ScopeDrawable.hpp"
#include "Media.hpp"
#include "tt/FrequencyAnalyzer.hpp"
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

	viz::ScopeDrawable<sf::RectangleShape> scope{{{}, (sf::Vector2i)size}};
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);
	scope.set_vert(true);
	scope.set_fill_in(true);

	viz::SpectrumDrawable<viz::VerticalBar> sd;
	sd.set_rect({{}, (sf::Vector2i)size});
	sd.set_bar_width(1);
	sd.set_bar_spacing(0);
	sd.set_color_mode(viz::SpectrumDrawable<viz::VerticalBar>::ColorMode::WHEEL);
	sd.set_color_wheel_rate(0.005);

	const auto fft_size = size.x;
	tt::FrequencyAnalyzer fa{fft_size};

	Media media{argv[3]};
	media.init(size);

	int afpvf{media._astream.sample_rate() / 60};

	std::vector<float> left_channel(size.x), spectrum(fft_size);

	pa::PortAudio _;
	pa::Stream pa_stream{0, media._astream.nb_channels(), paFloat32, media._astream.sample_rate()};
	pa_stream.start();

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
	
		{
			media.decode(size.x);

			if (media.audio_buffer.size() < size.x)
				break;

			// copy just the left channel
			for (int i = 0; i < size.x; ++i)
				left_channel[i] = media.audio_buffer[i * media._astream.nb_channels() + 0 /* left channel */];
			scope.update_shape_positions(left_channel);
			fa.copy_to_input(left_channel.data());
			fa.render(spectrum);
			sd.update_bar_heights(spectrum);
			sd.color_wheel_increment();
			

			try
			{
				pa_stream.write(media.audio_buffer.data(), afpvf);
			}
			catch (const pa::Error &e)
			{
				if (e.code != paOutputUnderflowed)
					throw;
				std::cerr << e.what() << '\n';
			}
			media.audio_buffer_erase(afpvf);
		}


		window.clear();
		window.draw(scope);
		window.draw(sd);
		window.display();
	}
}
