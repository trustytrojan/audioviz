#include <audioviz/Base.hpp>
#include <audioviz/media/FfmpegPopenEncoder.hpp>
#include <format>
#include <iostream>

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

bool Base::next_frame(const std::span<const float> audio_buffer)
{
	if (audio_buffer.size() / 2 < audio_frames_needed) // assuming stereo
	{
		std::cerr << "Base::next_frame: not enough audio frames, returning false\n";
		return false;
	}

	update(audio_buffer);

	final_rt.clear();
	for (auto &layer : layers)
		capture_time("layer '" + layer.get_name() + "'", layer.full_lifecycle(final_rt));
	final_rt.display();

	if (tt_enabled)
	{
		auto s = std::format("{:<20}{:<7}{:<7}{:<7}{:<7}\n", "", "curr", "avg", "min", "max");
		for (const auto &[label, stat] : timing_stats)
		{
			s += std::format(
				"{:<20}{:<7.3f}{:<7.3f}{:<7.3f}{:<7.3f}\n", label, stat.current, stat.avg(), stat.min, stat.max);
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
	if (tt_enabled)
		target.draw(timing_text);
}

void Base::capture_elapsed_time(const std::string &label, const sf::Clock &clock)
{
	auto &stat = timing_stats[label];
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

void Base::start_in_window(Media &media, const std::string &window_title)
{
	set_samplerate(media.audio_sample_rate());
	sf::RenderWindow window{
		sf::VideoMode{size},
		window_title,
		sf::Style::Titlebar,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	window.setVerticalSyncEnabled(true);

#ifdef AUDIOVIZ_PORTAUDIO
	pa::Init pa_init;
	pa::Stream pa_stream{0, media.audio_channels(), paFloat32, media.audio_sample_rate(), afpvf};
	pa_stream.start();
#endif

	while (window.isOpen())
	{
		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto samples = frames * media.audio_channels();
		media.buffer_audio(frames);
		if (media.audio_buffer_frames() < afpvf)
		{
			std::cerr << "Base::start_in_window: not enough audio frames, breaking loop\n";
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

		if (!next_frame(audio_chunk))
			break;

		// slide audio window just enough to ensure the next video
		// frame isn't reusing audio from this one
		media.audio_buffer_erase(afpvf);

		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
		window.clear();
		window.draw(*this);
		window.display();
	}
}

void Base::encode(Media &media, const std::string &outfile, const std::string &vcodec, const std::string &acodec)
{
	set_samplerate(media.audio_sample_rate());
	FfmpegPopenEncoder ffmpeg{media.url, *this, outfile, vcodec, acodec};
	RenderTexture rt{size, 4};

	bool running = true;
	while (running)
	{
		const auto frames = std::max(audio_frames_needed, afpvf);
		const auto samples = frames * media.audio_channels();
		media.buffer_audio(frames);
		if (media.audio_buffer_frames() < afpvf)
		{
			std::cerr << "Base::start_in_window: not enough audio frames, breaking loop\n";
			break;
		}
		const auto audio_chunk = std::span{media.audio_buffer()}.first(samples);

		running = next_frame(audio_chunk);
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
