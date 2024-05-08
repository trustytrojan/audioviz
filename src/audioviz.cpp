#include "audioviz.hpp"
#include <iostream>

audioviz::audioviz(const sf::Vector2u size, const std::string &media_url, const int antialiasing)
	: size(size),
	  _format(media_url),
	  ps(size, 50),
	  rt(size, antialiasing)
{
	set_text_defaults();

	// for now only stereo is supported
	if (_astream.nb_channels() != 2)
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

	try
	{
		auto _s = _format.find_best_stream(AVMEDIA_TYPE_VIDEO);
		// we don't want to re-decode the attached pic stream
		if (!(_s->disposition & AV_DISPOSITION_ATTACHED_PIC))
		{
			_vstream = _s;
			_vdecoder.emplace(avcodec_find_decoder(_s->codecpar->codec_id)).open();
			_scaler.emplace(av::nearest_multiple_8(_s->codecpar->width),
							_s->codecpar->height,
							(AVPixelFormat)_s->codecpar->format,
							size.x, size.y, AV_PIX_FMT_RGBA);
			_scaled_frame.emplace();
			_frame_queue.emplace();
		}
	}
	catch (const av::Error &e)
	{
		switch (e.errnum)
		{
		case AVERROR_STREAM_NOT_FOUND:
			std::cerr << "video stream not found\n";
			break;
		case AVERROR_DECODER_NOT_FOUND:
			std::cout << "video decoder not found\n";
			break;
		default:
			throw;
		}
	}

	// audio decoding initialization
	rs_frame->ch_layout = _astream->codecpar->ch_layout;
	rs_frame->sample_rate = _astream->codecpar->sample_rate;
	rs_frame->format = AV_SAMPLE_FMT_FLT;
	_adecoder.open();

	// FOR SOME REASON, THIS IS NECESSARY ON WEBM/OPUS CONTAINERS
	// OTHERWISE PACKET->PTS GOES UP WAY TOO FAST, CAUSING AN EARLY EOF
	_format.seek_file(-1, -1, 0, 0, AVSEEK_FLAG_ANY);
}

audioviz::_rt::_rt(const sf::Vector2u size, const int antialiasing)
	: spectrum(size, antialiasing),
	  particles(size, antialiasing),
	  bg(size, sf::ContextSettings(0, 0, antialiasing)) {}

audioviz::_rt::_blur::_blur(const sf::Vector2u size, const int antialiasing)
	: original(size, sf::ContextSettings(0, 0, antialiasing)),
	  blurred(size) {}

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
	// if image is non-square, choose a square textureRect in the center of the image
	auto tsize = (sf::Vector2i)album_cover.texture.getSize();
	if (tsize.x != tsize.y)
	{
		const auto square_size = std::min(tsize.x, tsize.y);
		album_cover.sprite.setTextureRect({{tsize.x / 2.f - square_size / 2.f, 0},
										   {square_size, square_size}});
	}
	else
		album_cover.sprite.setTextureRect({{}, tsize});

	tsize = album_cover.sprite.getTextureRect().getSize();

	// scale sprite to `scale_to`
	float scale_factor = std::min(scale_to.x / tsize.x, scale_to.y / tsize.y);
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

	auto ac_size = album_cover.sprite.getTextureRect().getSize();
	const auto ac_scale = album_cover.sprite.getScale();
	ac_size.x *= ac_scale.x;
	ac_size.y *= ac_scale.y;

	const sf::Vector2f text_pos{pos.x + ac_size.x + 10 * bool(ac_size.x), pos.y};
	title_text.setPosition({text_pos.x, text_pos.y});
	artist_text.setPosition({text_pos.x, text_pos.y + title_text.getCharacterSize() + 5});
}

void audioviz::set_text_defaults()
{
	if (const auto title = _astream.metadata("title"))
		title_text.setString(title);
	if (const auto title = _format.metadata("title"))
		title_text.setString(title);
	if (const auto artist = _astream.metadata("artist"))
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
	_afpvf = _astream.sample_rate() / framerate;
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
	ss.do_fft(audio_buffer.data());
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
	const auto speed_increase = sqrtf(size.y * avg) / 2;

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
	if (_vstream && !_frame_queue->empty())
	{
		rt.bg.draw(sf::Sprite(_frame_queue->front()));
		_frame_queue->pop_front();
		rt.bg.display();
		// rt.bg.blur(5, 5, 10); // anything > 10 is quite slow
		// rt.bg.mult(0.8);
	}

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
		pa_stream.emplace(0, 2, paFloat32, _astream.sample_rate(), sample_size);
		pa_stream->start();
	}
	else
	{
		pa_stream.reset();
		pa_init.reset();
	}
}

void audioviz::decode_media()
{
	// while we don't have enough audio samples
	while ((int)audio_buffer.size() < 2 * sample_size)
	{
		const auto packet = _format.read_packet();

		if (!packet)
		{
			std::cerr << "packet is null; format probably reached eof\n";
			return;
		}

		// const auto &stream = _format.streams[packet->stream_index];
		// std::cerr << stream->index << ": " << (packet->pts * av_q2d(stream->time_base)) << '/' << stream.duration_sec() << '\n';

		if (packet->stream_index == _astream->index)
		{
			if (!_adecoder.send_packet(packet))
			{
				std::cerr << "audio decoder has been flushed\n";
				continue;
			}

			while (const auto frame = _adecoder.receive_frame())
			{
				_resampler.convert_frame(rs_frame.get(), frame);
				const auto data = reinterpret_cast<const float *>(rs_frame->extended_data[0]);
				const auto nb_floats = 2 * rs_frame->nb_samples;
				audio_buffer.insert(audio_buffer.end(), data, data + nb_floats);
			}
		}
		else if (_vstream && packet->stream_index == _vstream->get()->index)
		{
			// we can access all of the video-related optionals in this block

			if (!_vdecoder->send_packet(packet))
			{
				std::cerr << "video decoder has been flushed\n";
				continue;
			}

			while (const auto frame = _vdecoder->receive_frame())
			{
				_scaler->scale_frame(_scaled_frame->get(), frame);
				// put a frame into an sf::Texture, then put that into a frame queue.
				// we are going to be reading more packets than usual since
				// we need more audio samples than is provided by one audio packet.

				// resize manully to use the newly allocated sf::Texture in-place
				_frame_queue->resize(_frame_queue->size() + 1);
				if (!_frame_queue->back().create({_scaled_frame->get()->width, _scaled_frame->get()->height}))
					throw std::runtime_error("failed to create texture!");
				_frame_queue->back().update(_scaled_frame->get()->data[0]);
			}
		}
	}
}

void audioviz::play_audio()
{
	try // to play the audio
	{
		pa_stream->write(audio_buffer.data(), _afpvf);
	}
	catch (const pa::Error &e)
	{
		if (e.code != paOutputUnderflowed)
			throw;
		std::cerr << e.what() << '\n';
	}
}

bool audioviz::draw_frame(sf::RenderTarget &target)
{
	decode_media();

	if (pa_stream)
		play_audio();

	// we don't have enough samples for fft; end here
	if ((int)audio_buffer.size() < 2 * sample_size)
		return false;

	draw_spectrum();
	blur_spectrum();

	if (framerate == 60)
		draw_particles();
	else if (ps_clock.getElapsedTime().asMilliseconds() > (1000.f / 60))
	{
		// keep the tickrate of the particles at 60hz for non-60fps output
		draw_particles();
		ps_clock.restart();
	}

	blur_particles();
	actually_draw_on_target(target);

	// THE IMPORTANT PART
	audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + 2 * _afpvf);

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