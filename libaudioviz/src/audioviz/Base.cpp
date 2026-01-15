#include <audioviz/Base.hpp>
#include <audioviz/media/FfmpegPopenEncoder.hpp>
#include <format>
#include <iostream>

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef AUDIOVIZ_IMGUI
#include <imgui-SFML.h>
#include <imgui.h>
#endif

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#define capture_time(label, code)            \
	if (tt_enabled)                          \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

namespace audioviz
{

Base::Base(const sf::Vector2u size)
	: size{size},
	  final_rt{size}
{
	timing_text.setPosition({size.x - 500, 30});
	timing_text.setCharacterSize(18);
	timing_text.setFillColor({255, 255, 255, 150});
}

bool Base::next_frame(const std::span<const float> audio_buffer, int num_channels)
{
	if (audio_buffer.size() < audio_frames_needed * num_channels) // assuming stereo
	{
		std::cerr << "[Base::next_frame] not enough audio frames, returning false\n";
		return false;
	}

	// capture_time("update", update(audio_buffer));
	update(audio_buffer);

	final_rt.clear();
	for (auto &layer : layers)
		capture_time("layer '" + layer.get_name() + "'", layer.full_lifecycle(final_rt));
	final_rt.display();

	if (tt_enabled)
	{
		auto s = std::format("{:<20}{:<7}{:<7}{:<7}{:<7}\n", "", "curr", "avg", "min", "max");
		for (const auto &stat : timing_stats)
		{
			s += std::format(
				"{:<20}{:<7.3f}{:<7.3f}{:<7.3f}{:<7.3f}\n", stat.name, stat.current, stat.avg(), stat.min, stat.max);
		}
		timing_text.setString(s);
	}

	return true;
}

void Base::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	target.draw(final_rt.sprite());
	for (const auto drawable : final_drawables)
		target.draw(*drawable);
	for (const auto dc : final_drawables2)
		target.draw(dc.drawable, dc.states);
	if (tt_enabled)
		target.draw(timing_text);
}

Base::TimingStat &Base::get_or_create_timing_stat(const std::string &label)
{
	auto it = std::ranges::find_if(timing_stats, [&label](auto &stat) { return stat.name == label; });
	if (it != timing_stats.end())
		return *it;
	return timing_stats.emplace_back(label);
}

void Base::capture_elapsed_time(const std::string &label, const sf::Clock &clock)
{
	auto &stat = get_or_create_timing_stat(label);
	const float time_ms = clock.getElapsedTime().asMicroseconds() / 1e3f;
	stat.current = time_ms;
	if (time_ms < stat.min)
		stat.min = time_ms;
	if (time_ms > stat.max)
		stat.max = time_ms;
	stat.total += time_ms;
	stat.count++;
}

void Base::set_framerate(const int framerate)
{
	this->framerate = framerate;
	afpvf = audio_sample_rate / framerate;
}

void Base::set_samplerate(const int samplerate)
{
	audio_sample_rate = samplerate;
	afpvf = audio_sample_rate / framerate;
}

Layer &Base::add_layer(const std::string &name, const int antialiasing)
{
	return layers.emplace_back(Layer{name, size, antialiasing});
}

Layer *Base::get_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](const auto &l) { return l.get_name() == name; });
	return (itr == layers.end()) ? nullptr : itr.base();
}

void Base::remove_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](const auto &l) { return l.get_name() == name; });
	if (itr != layers.end())
		layers.erase(itr);
}

void Base::add_final_drawable(const Drawable &d)
{
	final_drawables.emplace_back(&d);
}

void Base::add_final_drawable2(const Drawable &d, sf::RenderStates rs)
{
	final_drawables2.emplace_back(d, rs);
}

/*
TODO: You want to make libaudioviz more usage-agnostic by removing portaudio/imgui dependencies.
Libaudioviz by itself should be a machine that produces user-defined video from ambiguous audio.
Let client programs/callers/end-users handle the gathering of audio and whether drawables are editable.
*/

void Base::start_in_window(Media &media, const std::string &window_title)
{
	set_samplerate(media.audio_sample_rate());
	sf::RenderWindow window{
		sf::VideoMode{size},
		window_title,
		sf::Style::Titlebar | sf::Style::Close,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};

#ifdef AUDIOVIZ_IMGUI
	// Initialize ImGui-SFML
	if (!ImGui::SFML::Init(window))
	{
		std::cerr << "Failed to initialize ImGui-SFML\n";
		return;
	}
#endif

	const auto num_channels = media.audio_channels();

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
	pa::Stream pa_stream{0, num_channels, paFloat32, media.audio_sample_rate(), afpvf};
	pa_stream.start();

#ifdef __linux__
	// Restore stderr
	dup2(stderr_backup, STDERR_FILENO);
	close(stderr_backup);
#endif
#endif

	sf::Clock deltaClock;

	while (window.isOpen())
	{
		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto samples = frames * num_channels;
		media.buffer_audio(frames);
		if (media.audio_buffer_frames() < frames)
		{
			std::cerr << "[Base::start_in_window] not enough audio frames, breaking loop\n";
			break;
		}
		const auto audio_chunk = std::span{media.audio_buffer()}.first(samples);

#ifdef AUDIOVIZ_PORTAUDIO
		try
		{
			pa_stream.write(audio_chunk.data(), afpvf);
		}
		catch (const pa::Error &e)
		{
			if (e.code != paOutputUnderflowed)
				throw;
			std::cerr << e.what() << '\n';
		}
#endif

		// Process events
		while (const auto event = window.pollEvent())
		{
#ifdef AUDIOVIZ_IMGUI
			ImGui::SFML::ProcessEvent(window, *event);
#endif

			if (event->is<sf::Event::Closed>())
				window.close();
		}

#ifdef AUDIOVIZ_IMGUI
		ImGui::SFML::Update(window, deltaClock.restart());
#endif

		if (!next_frame(audio_chunk, num_channels))
			break;

		// slide audio window just enough to ensure the next video
		// frame isn't reusing audio from this one
		media.audio_buffer_erase(afpvf);

		window.clear();
		window.draw(*this);
#ifdef AUDIOVIZ_IMGUI
		ImGui::SFML::Render(window);
#endif
		window.display();
	}

#ifdef AUDIOVIZ_IMGUI
	ImGui::SFML::Shutdown();
#endif
}

void Base::encode(Media &media, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	set_samplerate(media.audio_sample_rate());
	// Create OpenGL context first (sf::RenderWindow usually does this for us) otherwise GL extensions will be null!
	sf::Context c;
	FfmpegPopenEncoder ffmpeg{media.url, *this, outfile, vcodec, acodec};
	RenderTexture rt{size, 4};
	const auto num_channels = media.audio_channels();

	bool running = true;
	while (running)
	{
		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto samples = frames * num_channels;
		media.buffer_audio(frames);
		if (media.audio_buffer_frames() < afpvf)
		{
			std::cerr << "Base::start_in_window: not enough audio frames, breaking loop\n";
			break;
		}
		const auto audio_chunk = std::span{media.audio_buffer()}.first(samples);

		running = next_frame(audio_chunk, num_channels);
		if (!running)
			break;

		// slide audio window just enough to ensure the next video
		// frame isn't reusing audio from this one
		media.audio_buffer_erase(afpvf);

		rt.clear();
		rt.draw(*this);
		rt.display();
		ffmpeg.send_frame(rt.getTexture());
	}
}

} // namespace audioviz
