#include <audioviz/Player.hpp>
#include <audioviz/media/FfmpegPopenEncoder.hpp>
#include <fcntl.h>
#include <iostream>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

namespace audioviz
{

Player::Player(Base &viz, Media &media, int framerate, int audio_frames_needed)
	: viz{viz},
	  media{media},
	  framerate{framerate},
	  audio_frames_needed{audio_frames_needed}
{
}

/*
TODO: You want to make libaudioviz more usage-agnostic by removing portaudio/imgui dependencies.
Libaudioviz by itself should be a machine that produces user-defined video from ambiguous audio.
Let client programs/callers/end-users handle the gathering of audio and whether drawables are editable.
*/

void Player::start_in_window(const std::string &title)
{
	sf::RenderWindow window{
		sf::VideoMode{viz.size},
		title,
		sf::Style::Titlebar | sf::Style::Close,
		sf::State::Windowed,
	};

#ifdef AUDIOVIZ_PORTAUDIO
#ifdef __linux__
	// Suppress ALSA warnings during PortAudio initialization
	int stderr_backup = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	dup2(devnull, STDERR_FILENO);
	close(devnull);
#endif

	// this looks like a bad idea, but letting portaudio handle timing
	// with it's blocking write function is better than constantly getting
	// "output underflowed" errors
	window.setVerticalSyncEnabled(false);
	window.setFramerateLimit(0);

	pa::Init pa_init;
	pa::Stream pa_stream{0, media.audio_channels(), paFloat32, media.audio_sample_rate(), afpvf};
	pa_stream.start();

#ifdef __linux__
	// Restore stderr
	dup2(stderr_backup, STDERR_FILENO);
	close(stderr_backup);
#endif
#endif

	while (window.isOpen())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();

		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto audio = media.read_audio(frames);

		if (!audio)
		{
			window.close();
			continue;
		}

#ifdef AUDIOVIZ_PORTAUDIO
		try
		{
			pa_stream.write(audio->data(), afpvf);
		}
		catch (const pa::Error &e)
		{
			if (e.code != paOutputUnderflowed)
				throw e;
			std::cerr << "PortAudio: Output underflowed\n";
		}
#endif

		viz.next_frame(*audio);

		// erase the audio "played" during this frame
		media.consume_audio(afpvf);

		window.clear();
		window.draw(viz);
		window.display();
	}
}

void Player::encode(const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	// Create OpenGL context first (sf::RenderWindow usually does this for us) otherwise GL extensions will be null!
	sf::Context c;
	sf::RenderTexture rt{viz.size};
	FfmpegPopenEncoder ffmpeg{media.url, viz.size, framerate, outfile, vcodec, acodec};
	while (true)
	{
		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto audio = media.read_audio(frames);

		if (!audio)
			break;

		viz.next_frame(*audio);

		// erase the audio "played" during this frame
		media.consume_audio(afpvf);

		rt.clear();
		rt.draw(viz);
		rt.display();
		ffmpeg.send_frame(rt.getTexture());
	}
}

} // namespace audioviz
