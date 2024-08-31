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

audioviz::audioviz(const sf::Vector2u size, const std::string &media_url, const int antialiasing)
	: size(size),
	  media(media_url),
	  ps({{}, (sf::Vector2i)size}, 50),
	  timing_text(font),
	  final_rt(size, antialiasing)
{
	// for now only stereo is supported
	if (media->_astream.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default margin around the spectrum
	set_spectrum_margin(ss_margin);

	timing_text.setPosition({size.x - 300, 30});
	timing_text.setCharacterSize(18);
	timing_text.setFillColor({255, 255, 255, 150});

	// initialize libav for media decoding
	media->init(size);

	metadata_init();

	// clang-format off

	// layer setup
	auto &bg = add_layer("bg", antialiasing);
	if (media->_vstream) // set_orig_cb() to draw video frames on the layer
	{
		std::cout << "media->_vstream\n";
		bg.set_orig_cb([&](auto &orig_rt) {
			if (media->_frame_queue->empty())
				return;
			// the frame queue should have frames scaled to this->size! see _media::decode()
			orig_rt.draw(sf::Sprite(media->_frame_queue->front()));
			media->_frame_queue->pop_front();
			orig_rt.display();
		});
		bg.set_fx_cb([](auto &, auto &fx_rt, auto &target) { target.draw(fx_rt.sprite); });
	}
	else // set_background() will apply_fx() on the bg it sets
	{
		// we only have one image; don't run effects in Layer::full_lifecycle
		bg.set_auto_fx(false);

		if (media->attached_pic)
		{
			std::cout << "media->attached_pic\n";
			_metadata.set_album_cover(*media->attached_pic, {150, 150});
			set_background(*media->attached_pic);
		}

		// don't set_fx_cb() if there is no video stream!
	}

	// default metadata position
	// needs to be called AFTER album cover is set because the text position depends on AC size
	_metadata.set_position({30, 30});

	auto &particles = add_layer("particles", antialiasing);
	particles.set_orig_cb([&](auto &orig_rt) {
		if (framerate <= 60)
		{
			capture_time("particles_update", ps.update(sa));
			orig_rt.clear(sf::Color::Transparent);
			capture_time("particles_draw", orig_rt.draw(ps));
			orig_rt.display();
		}
		else if (framerate > 60 && ps_clock.getElapsedTime().asMilliseconds() > (1000.f / 60))
		{
			// lock the tickrate of the particles at 60hz for >60fps output
			capture_time("particles_update", ps.update(sa));
			orig_rt.clear(sf::Color::Transparent);
			capture_time("particles_draw", orig_rt.draw(ps));
			orig_rt.display();
			ps_clock.restart();
		}
	});
	particles.set_fx_cb([&](auto &orig_rt, auto &fx_rt, auto &target) {
		target.draw(fx_rt.sprite, sf::BlendAdd);
		target.draw(orig_rt.sprite, sf::BlendAdd);
	});

	auto &spectrum = add_layer("spectrum", antialiasing);
	spectrum.set_orig_cb([&](auto &orig_rt) {
		capture_time("spectrum_update", ss.update(sa));
		orig_rt.clear(sf::Color::Transparent);
		capture_time("spectrum_draw", orig_rt.draw(ss));
		orig_rt.display();
	});
	spectrum.set_fx_cb([&](auto &orig_rt, auto &fx_rt, auto &target) {
		// BlendAdd is a sane default
		// experiment with other blend modes to make cool glows
		target.draw(fx_rt.sprite, sf::BlendAdd);

		if (spectrum_bm)
			target.draw(orig_rt.sprite, *spectrum_bm);
		else
			/**
			* since i haven't found the right blendmode that gets rid of the dark
			* spectrum bar edges, the default behavior (FOR NOW) is to redraw the spectrum.
			* users can pass --blendmode to instead blend the RT with the target above.
			*/
			target.draw(ss);
	});

	// clang-format on
}

viz::Layer &audioviz::add_layer(const std::string &name, int antialiasing)
{
	return layers.emplace_back(viz::Layer{name, size, antialiasing});
}

viz::Layer *audioviz::get_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](const auto &l) { return l.name == name; });
	if (itr == layers.end())
		return nullptr;
	return itr.base();
}

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
			bg->apply_fx();
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

void audioviz::set_timing_text_enabled(bool enabled)
{
	tt_enabled = enabled;
}

/* resizable windows not happening now, too much of the codebase relies on a static window size
void audioviz::set_size(const sf::Vector2u size)
{
	this->size = size;
	set_spectrum_margin(ss_margin);
}
*/

void audioviz::set_album_cover(const std::string &image_path, const sf::Vector2f size)
{
	sf::Texture txr;
	if (!txr.loadFromFile(image_path))
		throw std::runtime_error("failed to load album cover: '" + image_path + '\'');
	_metadata.set_album_cover(txr, size);
}

void audioviz::set_text_font(const std::string &path)
{
	if (!font.openFromFile(path))
		throw std::runtime_error("failed to load font: '" + path + '\'');
	font_loaded = true;
}

void audioviz::metadata_init()
{
	auto &title_text = _metadata.title_text;
	auto &artist_text = _metadata.artist_text;

	// set style, fontsize, and color
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);
	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});

	// set text using stream/format metadata
	_metadata.use_metadata(*media);
}

void audioviz::set_spectrum_margin(const int margin)
{
	ss_margin = margin;
	ss.set_rect({{margin, margin}, {size.x - 2 * margin, size.y - 2 * margin}});
}

void audioviz::set_framerate(int framerate)
{
	this->framerate = framerate;
	_afpvf = media->_astream.sample_rate() / framerate;
}

void audioviz::set_background(const sf::Texture &txr)
{
	const auto bg = get_layer("bg");
	if (!bg)
		throw std::runtime_error("no background layer present!");
	tt::Sprite spr(txr);

	// i do this because of *widescreen* youtube thumbnails containing *square* album covers
	spr.capture_centered_square_view();

	spr.fill_screen(size);
	bg->orig_draw(spr);
	bg->orig_display();
	bg->apply_fx();

	bg->set_fx_cb([](auto &, auto &fx_rt, auto &target) { target.draw(fx_rt.sprite); });
}

void audioviz::set_spectrum_blendmode(const sf::BlendMode &bm)
{
	spectrum_bm = bm;
}

void audioviz::capture_elapsed_time(const char *const label, const sf::Clock &_clock)
{
	tt_ss << std::setw(20) << std::left << label << _clock.getElapsedTime().asMicroseconds() / 1e3f << "ms\n";
}

// void audioviz::draw_spectrum()
// {
// 	capture_time("spectrum_update", ss.update(sa));
// 	spectrum.orig_clear();
// 	capture_time("spectrum_draw", spectrum.orig_draw(ss));
// 	capture_time("spectrum_fx", spectrum.apply_fx());
// }

// should be called AFTER draw_spectrum()
// void audioviz::draw_particles()
// {
// 	capture_time("particles_update", ps.update(sa));
// 	particles.orig_clear();
// 	capture_time("particles_draw", particles.orig_draw(ps));
// 	capture_time("particles_fx", particles.apply_fx());
// }

#ifdef AUDIOVIZ_PORTAUDIO
void audioviz::set_audio_playback_enabled(bool enabled)
{
	if (enabled)
	{
		pa_init.emplace();
		pa_stream.emplace(0, 2, paFloat32, media->_astream.sample_rate(), _afpvf);
		pa_stream->start();
	}
	else
	{
		pa_stream.reset();
		pa_init.reset();
	}
}
#endif

#ifdef AUDIOVIZ_PORTAUDIO
void audioviz::play_audio()
{
	try // to play the audio
	{
		pa_stream->write(media->audio_buffer.data(), _afpvf);
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
	// capture_time("media_decode", media->decode(*this));
	capture_time("media_decode", media->decode(fft_size));

#ifdef AUDIOVIZ_PORTAUDIO
	if (pa_stream)
		capture_time("play_audio", play_audio());
#endif

	// we don't have enough samples for fft; end here
	if ((int)media->audio_buffer.size() < 2 * fft_size)
		return false;

	// resizes sa's vectors properly to fit ss's bars
	ss.before_analyze(sa);

	// perform actual fft
	capture_time("fft", sa.analyze(fa, media->audio_buffer.data(), true));

	/* brighten bg on bass - looks kinda bad, keeping code for future reference
	if (bg.effects.size())
	{
		tt_clock.restart();
		const auto &left_data = sa.left_data(),
				&right_data = sa.right_data();
		assert(left_data.size() == right_data.size());

		// clang-format off
		const auto weight_func = [](auto x){ return powf(x, 1 / 8.f); };
		// clang-format on

		const auto avg = (viz::util::weighted_max(left_data, weight_func) + viz::util::weighted_max(right_data, weight_func)) / 2;

		// dynamic_cast<fx::Mult &>(*bg.effects[2]).factor = 1 + avg;
		// dynamic_cast<fx::Add &>(*bg.effects[3]).addend = 0.1 * avg;
		bg.apply_fx();
		tt_ss << "bg_brighten: " << tt_clock.getElapsedTime().asMicroseconds() / 1e3f << "ms\n";
	}
	*/

	final_rt.clear();
	for (auto &layer : layers)
		capture_time(layer.name.c_str(), layer.full_lifecycle(final_rt));
	final_rt.display();

	// THE IMPORTANT PART
	// capture_time("audio_buffer_erase", audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + 2 * _afpvf));
	capture_time("audio_buffer_erase", media->audio_buffer_erase(_afpvf));

	timing_text.setString(tt_ss.str());
	tt_ss.str("");

	return true;
}

void audioviz::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	// target.draw(bg.fx_rt().sprite);
	// target.draw(particles.fx_rt().sprite, sf::BlendAdd);
	// target.draw(particles.orig_rt().sprite, sf::BlendAdd);

	// // BlendAdd is a sane default
	// // experiment with other blend modes to make cool glows
	// target.draw(spectrum.fx_rt().sprite, sf::BlendAdd);

	// if (spectrum_bm)
	// 	target.draw(spectrum.orig_rt().sprite, *spectrum_bm);
	// else
	// 	/**
	// 	 * since i haven't found the right blendmode that gets rid of the dark
	// 	 * spectrum bar edges, the default behavior (FOR NOW) is to redraw the spectrum.
	// 	 * users can pass --blendmode to instead blend the RT with the target above.
	// 	 */
	// 	target.draw(ss);

	target.draw(final_rt.sprite);

	target.draw(_metadata);

	if (tt_enabled)
		target.draw(timing_text);
}

void audioviz::set_sample_size(int n)
{
	fa.set_fft_size(n);
}

void audioviz::set_bar_width(int width)
{
	ss.set_bar_width(width);
}

void audioviz::set_bar_spacing(int spacing)
{
	ss.set_bar_spacing(spacing);
}

void audioviz::set_color_mode(SD::ColorMode mode)
{
	ss.set_color_mode(mode);
}

void audioviz::set_solid_color(sf::Color color)
{
	ss.set_solid_color(color);
}

void audioviz::set_color_wheel_rate(float rate)
{
	ss.set_color_wheel_rate(rate);
}

void audioviz::set_color_wheel_hsv(sf::Vector3f hsv)
{
	ss.set_color_wheel_hsv(hsv);
}

void audioviz::set_multiplier(float multiplier)
{
	ss.set_multiplier(multiplier);
}

void audioviz::set_fft_size(int fft_size)
{
	fa.set_fft_size(fft_size);
}

void audioviz::set_interp_type(FS::InterpolationType interp_type)
{
	fa.set_interp_type(interp_type);
}

void audioviz::set_scale(FS::Scale scale)
{
	fa.set_scale(scale);
}

void audioviz::set_nth_root(int nth_root)
{
	fa.set_nth_root(nth_root);
}

void audioviz::set_accum_method(FS::AccumulationMethod method)
{
	fa.set_accum_method(method);
}

void audioviz::set_window_func(FS::WindowFunction wf)
{
	fa.set_window_func(wf);
}
