#include "Media3.hpp"
#include "tt/FrequencyAnalyzer.hpp"
#include "viz/StereoSpectrum.hpp"
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

	const sf::Vector2u size{std::stoi(argv[1]), std::stoi(argv[2])};
	sf::RenderWindow window{sf::VideoMode{size}, "ScopeDrawableTest"};
	window.setVerticalSyncEnabled(true);

	const auto framerate = 60;

	viz::StereoSpectrum<viz::VerticalBar> ss;
	ss.set_left_backwards(true);
	ss.set_rect({{}, (sf::Vector2i)size});
	ss.set_bar_width(10);
	ss.set_bar_spacing(5);

	// number of audio FRAMES needed for fft
	const auto fft_size = 3000;
	tt::FrequencyAnalyzer fa{fft_size};
	tt::StereoAnalyzer sa;

	Media3 media{argv[3]};
	const auto &astream = media.astream();

	// number of audio FRAMES per video frame
	const int afpvf{astream.sample_rate() / framerate};

	// number of audio SAMPLES per video frame
	const int aspvf{afpvf * astream.nb_channels()};

	// number of audio SAMPLES needed for fft
	const auto fft_samples = fft_size * astream.nb_channels();

	// remember this stores SAMPLES not FRAMES
	// one audio FRAME is nb_channels SAMPLES long
	std::vector<float> audio_buffer(fft_samples);

	pa::PortAudio _;
	pa::Stream pa_stream{0, astream.nb_channels(), paFloat32, astream.sample_rate(), afpvf};
	pa_stream.start();

	while (window.isOpen())
	{
		// handle events
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		// ensure we have at least fft_samples samples
		if (audio_buffer.size() < fft_samples)
		{
			float buf[fft_samples];
			if (media.read_audio_samples(buf, fft_samples) < fft_samples)
			{
				std::cout << "not enough audio for fft, breaking loop\n";
				break;
			}
			audio_buffer.insert(audio_buffer.end(), buf, buf + fft_samples);
		}

		ss.configure_analyzer(sa);
		sa.analyze(fa, audio_buffer.data(), true);
		ss.update(sa);

		try
		{
			pa_stream.write(audio_buffer.data(), afpvf);
		}
		catch (const pa::Error &e)
		{
			if (e.code != paOutputUnderflowed)
				throw;
			std::cerr << e.what() << '\n';
		}

		const auto begin = audio_buffer.begin();
		audio_buffer.erase(begin, begin + afpvf * astream.nb_channels());

		window.clear();
		window.draw(ss);
		window.display();
	}
}
