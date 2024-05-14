#include "audioviz.hpp"
#include <iostream>

audioviz::audioviz(const sf::Vector2u size, const std::string &media_url, const int antialiasing)
	: size(size),
	  _format(media_url),
	  ps(size, 50),
	  bg(size, antialiasing),
	  particles(size, antialiasing),
	  spectrum(size, antialiasing)
{
	text_init();

	// for now only stereo is supported
	if (_astream.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default margin around the spectrum
	set_margin(10);

	// default metadata position
	set_metadata_position({30, 30});

	// add default effects!
	bg.effects.emplace_back(new fx::Blur{10, 10, 20});
	particles.effects.emplace_back(new fx::Blur{1, 1, 10});
	spectrum.effects.emplace_back(new fx::Blur{1, 1, 20});

	// init libav stuff
	av_init();
}

void audioviz::av_init()
{
	// if an attached pic is in the format, use it for bg and album cover
	// clang-format off
	if (const auto itr = std::ranges::find_if(_format.streams(), [](const av::Stream &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
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
	if (!_astream->codecpar->ch_layout.order)
		// this check is necessary for .wav files with no channel order information
		_astream->codecpar->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
	rs_frame->ch_layout = _astream->codecpar->ch_layout;
	rs_frame->sample_rate = _astream->codecpar->sample_rate;
	rs_frame->format = AV_SAMPLE_FMT_FLT;
	_adecoder.copy_params(_astream->codecpar);
	_adecoder.open();

	// complete fucking bs!
	// no real solution for this!
	// will need to keep adding more cases as i test with more formats!
	switch (_adecoder->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_format.seek_file(-1, 1, 1, 1, AVSEEK_FLAG_FRAME);
		break;
	case AV_CODEC_ID_OPUS:
		if (!_vstream)
			_format.seek_file(-1, 0, 0, 0, AVSEEK_FLAG_BYTE);
		break;
	default:
		break;
	}
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
	album_cover.sprite.capture_centered_square_view();
	album_cover.sprite.set_size(scale_to);
}

void audioviz::set_text_font(const std::filesystem::path &path)
{
	if (!font.loadFromFile(path))
		throw std::runtime_error("failed to load font: '" + path.string() + '\'');
	font_loaded = true;
}

void audioviz::set_metadata_position(const sf::Vector2f pos)
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

void audioviz::text_init()
{
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);
	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});
	if (const auto title = _astream.metadata("title"))
		title_text.setString(title);
	if (const auto title = _format.metadata("title"))
		title_text.setString(title);
	if (const auto artist = _astream.metadata("artist"))
		artist_text.setString(artist);
	if (const auto artist = _format.metadata("artist"))
		artist_text.setString(artist);
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

void audioviz::set_background(const std::filesystem::path &image_path)
{
	sf::Texture txr;
	if (!txr.loadFromFile(image_path))
		throw std::runtime_error("failed to load background image: '" + image_path.string() + '\'');
	set_background(txr);
}

void audioviz::set_background(const sf::Texture &texture)
{
	tt::Sprite spr(texture);
	// if (bg_ccsv)
	spr.capture_centered_square_view();
	spr.fill_screen(size);

	bg.rt.orig.draw(spr);
	bg.rt.orig.display();
	bg.apply_fx();
}

void audioviz::draw_spectrum()
{
	ss.do_fft(audio_buffer.data());
	spectrum.rt.orig.clear(zero_alpha);
	spectrum.rt.orig.draw(ss);
	spectrum.rt.orig.display();
	spectrum.apply_fx();
}

// should be called AFTER draw_spectrum()
void audioviz::draw_particles()
{
	const auto &left_data = ss.left().data();
	const auto &right_data = ss.right().data();
	assert(left_data.size() == right_data.size());

	// only data in the range [0, amount) will be considered
	const auto amount = left_data.size() / 3.5f;

	const auto weighted_max = [](auto begin, auto end, auto weight_start)
	{
		float max_value = *begin;
		float total_distance = static_cast<float>(std::distance(weight_start, end));
		for (auto it = begin; it != end; ++it)
		{
			float weight = it < weight_start ? 1.0f : sqrt(std::distance(it, end) / total_distance);
			float value = *it * weight;
			if (value > max_value)
				max_value = value;
		}
		return max_value;
	};

	const auto calc_max = [&](const auto &vec)
	{
		const auto begin = vec.begin();
		// old behavior: non-weighted max
		// return *std::max_element(begin, begin + amount);
		return weighted_max(
			begin,
			begin + amount,
			begin + (amount / 2));
	};

	const auto avg = (calc_max(left_data) + calc_max(right_data)) / 2;
	const auto scaled_avg = size.y * avg;

	// the deciding factor in particle speed increase
	const auto speed_increase = sqrtf(scaled_avg / 5);

	ps.update({0, -speed_increase});

	particles.rt.orig.clear(zero_alpha);
	particles.rt.orig.draw(ps);
	particles.rt.orig.display();
	particles.apply_fx();
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

bool audioviz::prepare_frame()
{
	decode_media();

	if (pa_stream)
		play_audio();

	// we don't have enough samples for fft; end here
	if ((int)audio_buffer.size() < 2 * sample_size)
		return false;

	// get next frame from video if available
	if (_vstream && !_frame_queue->empty())
	{
		bg.rt.orig.draw(sf::Sprite(_frame_queue->front()));
		_frame_queue->pop_front();
		bg.rt.orig.display();
		bg.apply_fx();
	}

	draw_spectrum();

	if (framerate == 60)
		draw_particles();
	else if (ps_clock.getElapsedTime().asMilliseconds() > (1000.f / 60))
	{
		// keep the tickrate of the particles at 60hz for non-60fps output
		draw_particles();
		ps_clock.restart();
	}

	// THE IMPORTANT PART
	audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + 2 * _afpvf);

	return true;
}

void audioviz::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	target.draw(bg.rt.with_fx.sprite);
	target.draw(particles.rt.with_fx.sprite, sf::BlendAdd);
	target.draw(particles.rt.orig.sprite, sf::BlendAdd);

	// BlendAdd is a sane default
	// experiment with other blend modes to make cool glows
	target.draw(spectrum.rt.with_fx.sprite, sf::BlendAdd);

	// redraw spectrum due to anti-aliased edges retaining original background color
	target.draw(ss);

	// draw metadata if available (TODO: decide whether to apply effects on metadata too)
	if (album_cover.texture.getNativeHandle())
		target.draw(album_cover.sprite);
	if (font_loaded)
	{
		if (!title_text.getString().isEmpty())
			target.draw(title_text);
		if (!artist_text.getString().isEmpty())
			target.draw(artist_text);
	}
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