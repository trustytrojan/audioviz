#include <audioviz/StereoSpectrum.hpp>
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

	const sf::Vector2u size{std::stoi(argv[1]), std::stoi(argv[2])};
	sf::RenderWindow window{sf::VideoMode{size}, "ScopeDrawableTest"};
	window.setVerticalSyncEnabled(true);

	audioviz::ColorSettings color;
	const auto framerate = 60;

	audioviz::StereoSpectrum<audioviz::VerticalBar> ss{color};
	ss.set_left_backwards(true);
	ss.set_rect({{}, (sf::Vector2i)size});
	ss.set_bar_width(10);
	ss.set_bar_spacing(5);

	// number of audio FRAMES needed for fft
	const auto fft_size = 3000;
	audioviz::fft::FrequencyAnalyzer fa{fft_size};
	audioviz::fft::StereoAnalyzer sa;

	std::unique_ptr<audioviz::media::Media> media{audioviz::media::Media::create(argv[3])};

	// number of audio FRAMES per video frame
	const int afpvf{media->audio_sample_rate() / framerate};

	pa::PortAudio _;
	pa::Stream pa_stream{0, media->audio_channels(), paFloat32, media->audio_sample_rate()};
	pa_stream.start();

	int frames{};

	while (window.isOpen())
	{
		// handle events
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		// ensure we have at least fft_samples samples
		media->buffer_audio(fft_size);
		if (media->audio_buffer_frames() < fft_size)
		{
			std::cout << "not enough audio for fft, breaking loop\n";
			break;
		}

		ss.configure_analyzer(sa);
		sa.analyze(fa, media->audio_buffer().data(), true);
		ss.update(sa);

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

		window.clear();
		window.draw(ss);
		window.display();

		std::cerr << "\r\e[2Kframes: " << frames++;
	}
}
