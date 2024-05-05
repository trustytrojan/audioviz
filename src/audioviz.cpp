#include "audioviz.hpp"
#include <iostream>
#include "../deps/libavpp/include/av/Frame.hpp"

audioviz::audioviz(const sf::Vector2u size, const std::string &media_url, const int antialiasing)
	: size(size),
	  _format(media_url),
	  ps(size, 50),
	  rt(size, antialiasing)
{
	set_text_defaults();

	// for now only stereo is supported
	if (_stream.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default margin around the spectrum
	set_margin(10);

	// default metadata position
	set_metadata_position({30, 30});

	// if an attached pic is in the format, use it for bg and album cover
	// clang-format off
	if (const auto itr = std::ranges::find_if(_format.streams(), [](const av::Stream &s){ return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
		itr != _format.streams().cend())
	// clang-format on
	{
		const auto &stream = *itr;
		if (sf::Texture txr; txr.loadFromMemory(stream->attached_pic.data, stream->attached_pic.size))
		{
			set_background(txr);
			album_cover.texture = txr;
			_set_album_cover();
		}
	}
}

audioviz::_rt::_rt(const sf::Vector2u size, const int antialiasing)
	: spectrum(size, antialiasing),
	  particles(size, antialiasing),
	  bg(size, sf::ContextSettings(0, 0, antialiasing)) {}

audioviz::_rt::_blur::_blur(const sf::Vector2u size, const int antialiasing)
	: original(size, sf::ContextSettings(0, 0, antialiasing)),
	  blurred(size) {}

const std::span<float> &audioviz::current_audio() const
{
	return audio_span;
}

void audioviz::decoder_thread_func()
{
	pthread_setname_np(pthread_self(), "DecoderThread");
	av::Frame rs_frame;
	rs_frame->ch_layout = _stream->codecpar->ch_layout;
	rs_frame->sample_rate = _stream->codecpar->sample_rate;
	rs_frame->format = AV_SAMPLE_FMT_FLT;
	_decoder.open();
	while (const auto packet = _format.read_packet())
	{
		if (packet->stream_index != _stream->index)
			continue;
		if (!_decoder.send_packet(packet))
			break;
		while (const auto frame = _decoder.receive_frame())
		{
			_resampler.convert_frame(rs_frame.get(), frame);
			const auto data = reinterpret_cast<const float *>(rs_frame->extended_data[0]);
			const auto nb_floats = 2 * rs_frame->nb_samples;
			ab.buffer().insert(ab.buffer().end(), data, data + nb_floats);
		}
	}
	std::cout << "decoding finished\n";
}

bool audioviz::decoder_thread_finished()
{
	return decoder_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void audioviz::set_title_text(const std::string &text)
{
	title_text.setString(text);
}

void audioviz::set_artist_text(const std::string &text)
{
	artist_text.setString(text);
}

void audioviz::set_album_cover(const std::filesystem::path &image_path, const sf::Vector2f scale_to)
{
	if (!album_cover.texture.loadFromFile(image_path))
		throw std::runtime_error("failed to load album cover: '" + image_path.string() + '\'');
	_set_album_cover(scale_to);
}

void audioviz::_set_album_cover(const sf::Vector2f scale_to)
{
	album_cover.sprite.setTexture(album_cover.texture, true);

	// using `album_cover.texture`'s size, set the scale on `album_cover.sprite`
	// such that it will only take up a certain area
	// widescreen images will end up looking small but it's fine
	const auto ac_tsize = album_cover.texture.getSize();
	float scale_factor = std::min(scale_to.x / ac_tsize.x, scale_to.y / ac_tsize.y);
	album_cover.sprite.setScale({scale_factor, scale_factor});
}

void audioviz::set_text_font(const std::filesystem::path &path)
{
	if (!font.loadFromFile(path))
		throw std::runtime_error("failed to load font: '" + path.string() + '\'');
}

void audioviz::set_metadata_position(const sf::Vector2f &pos)
{
	album_cover.sprite.setPosition(pos);

	auto ac_size = album_cover.texture.getSize();
	const auto ac_scale = album_cover.sprite.getScale();
	ac_size.x *= ac_scale.x;
	ac_size.y *= ac_scale.y;

	const sf::Vector2f text_pos{pos.x + ac_size.x + 10, pos.y};
	title_text.setPosition({text_pos.x, text_pos.y});
	artist_text.setPosition({text_pos.x, text_pos.y + title_text.getCharacterSize() + 5});
}

void audioviz::set_text_defaults()
{
	if (const auto title = _stream.metadata("title"))
		title_text.setString(title);
	if (const auto title = _format.metadata("title"))
		title_text.setString(title);
	if (const auto artist = _stream.metadata("artist"))
		artist_text.setString(artist);
	if (const auto artist = _format.metadata("artist"))
		artist_text.setString(artist);
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);
	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});
}

void audioviz::set_margin(const int margin)
{
	ss.set_target_rect({{margin, margin}, {size.x - 2 * margin, size.y - 2 * margin}});
}

void audioviz::set_framerate(int framerate)
{
	this->framerate = framerate;
	afpvf = _stream.sample_rate() / framerate;
}

void audioviz::set_background(const std::filesystem::path &image_path, const EffectOptions opts)
{
	sf::Texture txr;
	if (!txr.loadFromFile(image_path))
		throw std::runtime_error("failed to load background image: '" + image_path.string() + '\'');
	set_background(txr, opts);
}

void audioviz::set_background(const sf::Texture &texture, const EffectOptions opts)
{
	sf::Sprite sprite(texture);

	// make sure bgTexture fills up the whole screen, and is centered
	const auto tsize = texture.getSize();
	sprite.setOrigin({tsize.x / 2, tsize.y / 2});
	sprite.setPosition({size.x / 2, size.y / 2});
	const auto scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
	sprite.setScale({scale, scale});

	rt.bg.draw(sprite);
	rt.bg.display();

	if (opts.blur.hrad && opts.blur.vrad && opts.blur.n_passes)
		rt.bg.blur(opts.blur.hrad, opts.blur.vrad, opts.blur.n_passes);
	if (opts.mult)
		rt.bg.mult(opts.mult);
}

void audioviz::draw_spectrum()
{
	ss.do_fft(audio_span.data());
	rt.spectrum.original.clear(zero_alpha);
	rt.spectrum.original.draw(ss);
	rt.spectrum.original.display();
}

// should be called AFTER draw_spectrum()
void audioviz::draw_particles()
{
	const auto left_data = ss.left().data(),
			   right_data = ss.right().data();
	assert(left_data.size() == right_data.size());

	// first quarter of the spectrum is generally bass
	const auto amount = left_data.size() / 4.f;

	const auto left_max = *std::max_element(left_data.begin(), left_data.begin() + amount);
	const auto right_max = *std::max_element(right_data.begin(), right_data.begin() + amount);

	const auto avg = (left_max + right_max) / 2;

	// cbrt to ease the sudden-ness of the particles' speed increase
	const auto speed_increase = cbrtf(size.y * avg);

	rt.particles.original.clear(zero_alpha);
	ps.draw(rt.particles.original, {0, -speed_increase});
	rt.particles.original.display();
}

void audioviz::blur_spectrum()
{
	rt.spectrum.blurred.clear(zero_alpha);
	rt.spectrum.blurred.draw(rt.spectrum.original.sprite());
	rt.spectrum.blurred.display();
	rt.spectrum.blurred.blur(1, 1, 20);
}

void audioviz::blur_particles()
{
	rt.particles.blurred.clear(zero_alpha);
	rt.particles.blurred.draw(rt.particles.original.sprite());
	rt.particles.blurred.display();
	rt.particles.blurred.blur(1, 1, 10);
}

void audioviz::actually_draw_on_target(sf::RenderTarget &target)
{
	// layer everything together
	target.draw(rt.bg.sprite());
	target.draw(rt.particles.blurred.sprite(), sf::BlendAdd);
	target.draw(rt.particles.original.sprite(), sf::BlendAdd);

	// BlendAdd is a sane default
	// experiment with other blend modes to make cool glows
	target.draw(rt.spectrum.blurred.sprite(), sf::BlendAdd);

	// redraw spectrum due to anti-aliased edges retaining original background color
	target.draw(ss.left());
	target.draw(ss.right());

	// draw metadata
	if (album_cover.texture.getNativeHandle())
		target.draw(album_cover.sprite);
	target.draw(title_text);
	target.draw(artist_text);
}

void audioviz::set_audio_playback_enabled(bool enabled)
{
	if (enabled)
	{
		pa_init.emplace();
		pa_stream.emplace(0, 2, paFloat32, _stream.sample_rate(), sample_size);
		pa_stream->start();
	}
	else
	{
		pa_stream.reset();
		pa_init.reset();
	}
}

bool audioviz::draw_frame(sf::RenderTarget &target)
{
	// wait for enough audio to be decoded
	// or, if decoding finished, just use what we can get
	while (!decoder_thread_finished() && (int)ab.frames_available() < sample_size)
		;

	// get frames using a span to avoid a copy
	const int frames_read = ab.read_frames(audio_span, sample_size);

	// nothing left, we are done
	if (!frames_read)
		return false;

	if (pa_stream)
		try // to play the audio
		{
			pa_stream->write(audio_span.data(), afpvf);
		}
		catch (const pa::Error &e)
		{
			if (e.code != paOutputUnderflowed)
				throw;
			std::cerr << e.what() << '\n';
		}

	// stop if we don't have enough samples to produce a frame
	if (frames_read != sample_size)
		return false;

	draw_spectrum();
	blur_spectrum();

	if (framerate == 60)
		draw_particles();
	else if (ps_clock.getElapsedTime().asMilliseconds() > (1000.f / 60.f))
	{
		// keep the tickrate of the particles at 60hz for non-60fps output
		draw_particles();
		ps_clock.restart();
	}

	blur_particles();

	actually_draw_on_target(target);

	// seek audio backwards
	ab.seek(afpvf - sample_size, SEEK_CUR);

	return true;
}

void audioviz::set_bar_width(int width)
{
	ss.left().bar.set_width(width);
	ss.right().bar.set_width(width);
}

void audioviz::set_bar_spacing(int spacing)
{
	ss.left().bar.set_spacing(spacing);
	ss.right().bar.set_spacing(spacing);
}

void audioviz::set_color_mode(SD::ColorMode mode)
{
	ss.left().color.set_mode(mode);
	ss.right().color.set_mode(mode);
}

void audioviz::set_solid_color(sf::Color color)
{
	ss.left().color.set_solid_rgb(color);
	ss.right().color.set_solid_rgb(color);
}

void audioviz::set_color_wheel_rate(float rate)
{
	ss.left().color.wheel.set_rate(rate);
	ss.right().color.wheel.set_rate(rate);
}

void audioviz::set_color_wheel_hsv(sf::Vector3f hsv)
{
	ss.left().color.wheel.set_hsv(hsv);
	ss.right().color.wheel.set_hsv(hsv);
}

void audioviz::set_multiplier(float multiplier)
{
	ss.left().set_multiplier(multiplier);
	ss.right().set_multiplier(multiplier);
}

// void audioviz::set_fft_size(int fft_size)
// {
// 	ss.left().set_fft_size(fft_size);
// 	ss.right().set_fft_size(fft_size);
// }

void audioviz::set_interp_type(FS::InterpolationType interp_type)
{
	ss.left().set_interp_type(interp_type);
	ss.right().set_interp_type(interp_type);
}

void audioviz::set_scale(FS::Scale scale)
{
	ss.left().set_scale(scale);
	ss.right().set_scale(scale);
}

void audioviz::set_nth_root(int nth_root)
{
	ss.left().set_nth_root(nth_root);
	ss.right().set_nth_root(nth_root);
}

void audioviz::set_accum_method(FS::AccumulationMethod method)
{
	ss.left().set_accum_method(method);
	ss.right().set_accum_method(method);
}

void audioviz::set_window_func(FS::WindowFunction wf)
{
	ss.left().set_window_func(wf);
	ss.right().set_window_func(wf);
}