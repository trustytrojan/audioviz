#include <audioviz/Base.hpp>

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

Base::Base(const sf::Vector2u size, media::Media *const media)
	: size{size},
	  media{media},
	  final_rt{size}
{
	assert(media && this->media);
	timing_text.setPosition({size.x - 300, 30});
	timing_text.setCharacterSize(18);
	timing_text.setFillColor({255, 255, 255, 150});
	set_timing_text_enabled(true);
}

Base::~Base()
{
	delete media;
}

#ifdef AUDIOVIZ_PORTAUDIO
void Base::set_audio_playback_enabled(const bool enabled)
{
	if (enabled)
	{
		audio_enabled = true;
		pa_stream.start();
	}
	else
	{
		audio_enabled = false;
		pa_stream.stop();
	}
}

void Base::play_audio()
{
	try // to play the audio
	{
		pa_stream.write(media->audio_buffer().data(), afpvf);
	}
	catch (const pa::Error &e)
	{
		if (e.code != paOutputUnderflowed)
			throw;
		std::cerr << e.what() << '\n';
	}
}
#endif

bool Base::next_frame()
{
	assert(media);
	media->decode_audio(std::max(audio_frames_needed, afpvf));

#ifdef AUDIOVIZ_PORTAUDIO
	// clang-format off
	if (audio_enabled && media->audio_buffer_frames() >= afpvf)
	{
		capture_time("play_audio", play_audio());
	}
	// clang-format on
#endif

	if (media->audio_buffer_frames() < audio_frames_needed)
		return false;

	final_rt.clear();
	for (auto &layer : layers)
		capture_time("layer '" + layer.get_name() + "'", layer.full_lifecycle(final_rt));
	final_rt.display();

	// THE IMPORTANT PART: erase already consumed audio
	media->audio_buffer_erase(afpvf);

	if (tt_enabled)
	{
		timing_text.setString(tt_ss.str());
		tt_ss.str("");
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
	tt_ss << std::setw(20) << std::left << label << clock.getElapsedTime().asMicroseconds() / 1e3f << "ms\n";
}

void Base::set_framerate(const int framerate)
{
	this->framerate = framerate;
	afpvf = media->astream().sample_rate() / framerate;
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
	final_drawables.emplace_back(std::addressof(d));
}

void Base::perform_fft(fft::FrequencyAnalyzer &fa, fft::AudioAnalyzer &aa)
{
	capture_time("fft", aa.analyze(fa, media->audio_buffer().data(), true));
}

void Base::start_in_window(const std::string &window_title)
{
#ifdef AUDIOVIZ_PORTAUDIO
	set_audio_playback_enabled(true);
#endif
	sf::RenderWindow window{
		sf::VideoMode{size},
		window_title,
		sf::Style::Titlebar,
		sf::State::Windowed,
		{.antiAliasingLevel = 4},
	};
	window.setVerticalSyncEnabled(true);
	while (window.isOpen() && next_frame())
	{
		while (const auto event = window.pollEvent())
			if (event->is<sf::Event::Closed>())
				window.close();
		window.draw(*this);
		window.display();
	}
}

} // namespace audioviz
