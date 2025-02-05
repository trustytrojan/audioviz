#include "audioviz.hpp"

#define capture_time(label, code)            \
	if (tt_enabled)                          \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

audioviz::audioviz(const sf::Vector2u size, media::Media *const media)
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

#ifdef AUDIOVIZ_PORTAUDIO
void audioviz::set_audio_playback_enabled(const bool enabled)
{
	if (enabled)
	{
		pa_init.emplace();
		pa_stream.emplace(0, 2, paFloat32, media->astream().sample_rate(), afpvf);
		pa_stream->start();
	}
	else
	{
		pa_stream.reset();
		pa_init.reset();
	}
}

void audioviz::play_audio()
{
	try // to play the audio
	{
		pa_stream->write(media->audio_buffer().data(), afpvf);
	}
	catch (const pa::Error &e)
	{
		if (e.code != paOutputUnderflowed)
			throw;
		std::cerr << e.what() << '\n';
	}
}
#endif

bool audioviz::next_frame()
{
	assert(media);
	media->decode_audio(std::max(audio_frames_needed, afpvf));

#ifdef AUDIOVIZ_PORTAUDIO
	if (pa_stream && media->audio_buffer_frames() >= afpvf)
		capture_time("play_audio", play_audio());
#endif

	if (media->audio_buffer_frames() < audio_frames_needed)
		return false;

	final_rt.clear();
	for (auto &layer : layers)
		capture_time("layer '" + layer.get_name() + "'", layer.full_lifecycle(final_rt));
	final_rt.display();

	// THE IMPORTANT PART: erase already consumed audio
	capture_time("audio_buffer_erase", media->audio_buffer_erase(afpvf));

	if (tt_enabled)
	{
		timing_text.setString(tt_ss.str());
		tt_ss.str("");
	}

	return true;
}

void audioviz::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	target.draw(final_rt.sprite());
	if (tt_enabled)
		target.draw(timing_text);
}

void audioviz::capture_elapsed_time(const std::string &label, const sf::Clock &clock)
{
	tt_ss << std::setw(20) << std::left << label << clock.getElapsedTime().asMicroseconds() / 1e3f << "ms\n";
}

void audioviz::set_framerate(const int framerate)
{
	this->framerate = framerate;
	afpvf = media->astream().sample_rate() / framerate;
}

viz::Layer &audioviz::add_layer(const std::string &name, const int antialiasing)
{
	return layers.emplace_back(viz::Layer{name, size, antialiasing});
}

viz::Layer *audioviz::get_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](const auto &l) { return l.get_name() == name; });
	return (itr == layers.end()) ? nullptr : itr.base();
}

void audioviz::remove_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](const auto &l) { return l.get_name() == name; });
	if (itr != layers.end())
		layers.erase(itr);
}

void audioviz::perform_fft(tt::FrequencyAnalyzer &fa, tt::AudioAnalyzer &aa)
{
	sf::Clock clock;
	aa.analyze(fa, media->audio_buffer().data(), true);
	capture_elapsed_time("fft", clock);
}
