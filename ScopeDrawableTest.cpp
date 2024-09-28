#include "viz/ScopeDrawable.hpp"
#include "Media.hpp"
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

	viz::ScopeDrawable<sf::RectangleShape> scope{{{}, (sf::Vector2i)size}};
	scope.set_shape_spacing(0);
	scope.set_shape_width(1);

	Media media{argv[3]};
	media.init(size);

	std::vector<float> left_channel;
	left_channel.resize(size.x);

	pa::PortAudio _;
	pa::Stream pa_stream{0, media._astream.nb_channels(), paFloat32, media._astream.sample_rate()};
	pa_stream.start();

	int i = 0;
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
			scope.update_shape_positions(media.audio_buffer);
			try
			{
				pa_stream.write(media.audio_buffer.data(), size.x);
			}
			catch (const pa::Error &e)
			{
				if (e.code != paOutputUnderflowed)
					throw;
				std::cerr << e.what() << '\n';
			}
			media.audio_buffer_erase(size.x);
		}

		window.clear();
		window.draw(scope);
		window.display();
	}
}
