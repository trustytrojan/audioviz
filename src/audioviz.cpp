#include <iomanip>
#include <iostream>

#include "audioviz.hpp"
#include "fx/Blur.hpp"
#include "fx/Mult.hpp"

#define capture_time(label, code)            \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}

audioviz::audioviz(
	const sf::Vector2u size,
	const std::string &media_url,
	tt::FrequencyAnalyzer &fa,
	viz::StereoSpectrum<BarType> &ss,
	viz::ParticleSystem<ParticleShapeType> &ps,
	const int antialiasing)
	: size{size},
	  media{media_url},
	  fa{fa},
	  ss{ss},
	  ps{ps},
	  final_rt{size, antialiasing}
{
	// for now only stereo is supported
	// this will be moved into its own class eventually
	if (media->_astream.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default spectrum margin
	// this sets the StereoSpectrum's rectangle (necessary for it to render)
	set_spectrum_margin(10);

	// create stereo "mirror" effect
	ss.set_left_backwards(true);

	timing_text.setPosition({size.x - 300, 30});
	timing_text.setCharacterSize(18);
	timing_text.setFillColor({255, 255, 255, 150});
	set_timing_text_enabled(true);

	media->init(size);
	metadata_init();
	layers_init(antialiasing);

	// default metadata position
	// needs to be called AFTER album cover is set because the text position depends on album cover size
	metadata.set_position({30, 30});
}

void audioviz::reset()
{
	media->reset();
}

void audioviz::perform_fft()
{
	ss.configure_analyzer(sa);
	capture_time("fft", sa.analyze(fa, media->audio_buffer.data(), true));
}

void audioviz::layers_init(const int antialiasing)
{
	{ // bg layer
		auto &bg = add_layer("bg", antialiasing);

		if (media->_vstream) // set_orig_cb() to draw video frames on the layer
		{
			const auto vfr = av_q2d(media->_vstream->get()->avg_frame_rate);
			const auto frames_to_wait = framerate / vfr;
			bg.set_orig_cb(
				[&](auto &orig_rt)
				{
					if (media->_frame_queue->empty() || vfcount < frames_to_wait)
						++vfcount;
					else
					{
						// the frame queue should have frames scaled to this->size! see Media::decode()
						orig_rt.draw(sf::Sprite(media->_frame_queue->front()));
						media->_frame_queue->pop_front();
						vfcount = 0;
					}
					orig_rt.display();
				});
			bg.set_fx_cb(viz::Layer::DRAW_FX_RT);
		}
		else // set_background() will apply_fx() on the bg it sets
		{
			// we only have one image; don't run effects in Layer::full_lifecycle
			bg.set_auto_fx(false);

			if (media->attached_pic)
			{
				metadata.set_album_cover(*media->attached_pic, {150, 150});
				set_background(*media->attached_pic);
			}

			// don't set_fx_cb() if there is no video stream!
		}
	}

	{ // particles layer
		auto &particles = add_layer("particles", antialiasing);
		particles.set_orig_cb(
			[&](auto &orig_rt)
			{
				// this HAS to be done before particles or spectrum, as both depend
				// on fft being performed on the current audio buffer for this frame
				perform_fft();

				// lock the tickrate of the particles at 60hz for non-60fps output

				if (framerate < 60)
					ps.update(sa, {.multiplier = 60.f / framerate});
				else if (framerate == 60)
					ps.update(sa);
				else if (framerate > 60 && frame_count >= (framerate / 60.))
				{
					ps.update(sa);
					frame_count = 0;
				}

				++frame_count;

				orig_rt.clear(sf::Color::Transparent);
				orig_rt.draw(ps);
				orig_rt.display();
			});
		particles.set_fx_cb(
			[&](auto &orig_rt, auto &fx_rt, auto &target)
			{
				target.draw(fx_rt.sprite(), sf::BlendAdd);
				target.draw(orig_rt.sprite(), sf::BlendAdd);
			});
	}

	{ // spectrum layer
		auto &spectrum = add_layer("spectrum", antialiasing);
		spectrum.set_orig_cb(
			[&](auto &orig_rt)
			{
				ss.update(sa);
				orig_rt.clear(sf::Color::Transparent);
				orig_rt.draw(ss);
				orig_rt.display();
			});
		spectrum.set_fx_cb(
			[&](auto &orig_rt, auto &fx_rt, auto &target)
			{
				target.draw(fx_rt.sprite(), sf::BlendAdd);

				if (spectrum_bm)
					target.draw(orig_rt.sprite(), *spectrum_bm);
				else
					/**
					 * since i haven't found the right blendmode that gets rid of the dark
					 * spectrum bar edges, the default behavior (FOR NOW) is to redraw the spectrum.
					 * users can pass --blendmode to instead blend the RT with the target above.
					 */
					target.draw(ss);
			});
	}
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

// need to do this outside of the constructor otherwise the texture is broken?
void audioviz::use_attached_pic_as_bg()
{
	if (media->attached_pic)
		set_background(*media->attached_pic);
}

void audioviz::add_default_effects()
{
	if (const auto bg = get_layer("bg"))
	{
		// clang-format off
		bg->effects.emplace_back(
			media->_vstream
				? new fx::Blur{2.5, 2.5, 5}
				: new fx::Blur{7.5, 7.5, 15});
		// clang-format on
		if (!media->_vstream)
			bg->effects.emplace_back(new fx::Mult{0.75});
		if (media->attached_pic)
			// this will reapply the effects without any bs
			set_background(*media->attached_pic);
	}

	if (const auto particles = get_layer("particles"))
		particles->effects.emplace_back(new fx::Blur{1, 1, 10});

	if (const auto spectrum = get_layer("spectrum"))
		spectrum->effects.emplace_back(new fx::Blur{1, 1, 20});
}

void audioviz::set_media_url(const std::string &url)
{
	media.emplace(url);
	media->init(size);
}

const std::string &audioviz::get_media_url() const
{
	return media->url;
}

void audioviz::set_timing_text_enabled(const bool enabled)
{
	tt_enabled = enabled;
}

void audioviz::set_album_cover(const std::string &image_path, const sf::Vector2f size)
{
	metadata.set_album_cover(sf::Texture{image_path}, size);
}

void audioviz::set_text_font(const std::string &path)
{
	font = {path};
}

void audioviz::metadata_init()
{
	// set style, fontsize, and color
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	title_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});

	artist_text.setStyle(sf::Text::Italic);
	artist_text.setCharacterSize(24);
	artist_text.setFillColor({255, 255, 255, 150});

	// set text using stream/format metadata
	metadata.use_metadata(*media);
}

void audioviz::set_spectrum_margin(const int margin)
{
	ss.set_rect({{margin, margin}, {size.x - 2 * margin, size.y - 2 * margin}});
}

void audioviz::set_framerate(const int framerate)
{
	this->framerate = framerate;
	afpvf = media->_astream.sample_rate() / framerate;
}

void audioviz::set_background(const sf::Texture &txr)
{
	const auto bg = get_layer("bg");
	if (!bg)
		throw std::runtime_error("no background layer present!");
	tt::Sprite spr{txr};

	// i do this because of *widescreen* youtube thumbnails containing *square* album covers
	spr.capture_centered_square_view();

	spr.fill_screen(size);
	bg->orig_draw(spr);
	bg->orig_display();
	bg->apply_fx();

	bg->set_fx_cb(viz::Layer::DRAW_FX_RT);
}

void audioviz::set_spectrum_blendmode(const sf::BlendMode &bm)
{
	spectrum_bm = bm;
}

void audioviz::capture_elapsed_time(const std::string &label, const sf::Clock &_clock)
{
	tt_ss << std::setw(20) << std::left << label << _clock.getElapsedTime().asMicroseconds() / 1e3f << "ms\n";
}

#ifdef AUDIOVIZ_PORTAUDIO
void audioviz::set_audio_playback_enabled(const bool enabled)
{
	if (enabled)
	{
		pa_init.emplace();
		pa_stream.emplace(0, 2, paFloat32, media->_astream.sample_rate(), afpvf);
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
		pa_stream->write(media->audio_buffer.data(), afpvf);
	}
	catch (const pa::Error &e)
	{
		if (e.code != paOutputUnderflowed)
			throw;
		std::cerr << e.what() << '\n';
	}
}
#endif

bool audioviz::prepare_frame()
{
	assert(media);
	capture_time("media_decode", media->decode(fft_size));

#ifdef AUDIOVIZ_PORTAUDIO
	if (pa_stream)
		capture_time("play_audio", play_audio());
#endif

	// we don't have enough samples for fft; end here
	if ((int)media->audio_buffer.size() < 2 * fft_size)
		return false;

	final_rt.clear();
	for (auto &layer : layers)
		capture_time(layer.get_name(), layer.full_lifecycle(final_rt));
	final_rt.display();

	// THE IMPORTANT PART
	capture_time("audio_buffer_erase", media->audio_buffer_erase(afpvf));

	timing_text.setString(tt_ss.str());
	tt_ss.str("");

	return true;
}

void audioviz::draw(sf::RenderTarget &target, const sf::RenderStates states) const
{
	target.draw(final_rt.sprite(), states);
	target.draw(metadata, states);
	if (tt_enabled)
		target.draw(timing_text, states);
}

void audioviz::set_fft_size(const int n)
{
	fft_size = n;
	fa.set_fft_size(n);
}
