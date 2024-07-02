#include <iostream>

#include "audioviz.hpp"
#include "viz/util.hpp"

audioviz::audioviz(const sf::Vector2u size, const std::string &media_url, const int antialiasing)
	: url(media_url),
	  size(size),
	  _format(media_url),
	  ps({{}, (sf::Vector2i)size}, 50),
	  bg(size, antialiasing),
	  particles(size, antialiasing),
	  spectrum(size, antialiasing)
{
	metadata_init();

	// for now only stereo is supported
	if (_astream.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default margin around the spectrum
	set_margin(10);

	// add default effects!
	bg.effects.emplace_back(new fx::Blur{7.5, 7.5, 15});
	particles.effects.emplace_back(new fx::Blur{1, 1, 10});
	spectrum.effects.emplace_back(new fx::Blur{1, 1, 20});

	// init libav stuff
	av_init();

	// default metadata position
	_metadata.set_position({30, 30});
}

void audioviz::av_init()
{
	// if an attached pic is in the format, use it for bg and album cover
	// clang-format off
	if (const auto itr = std::ranges::find_if(_format.streams(),
			[](const auto &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
		itr != _format.streams().cend())
	// clang-format on
	{
		const auto &stream = *itr;
		if (sf::Texture txr; txr.loadFromMemory(stream->attached_pic.data, stream->attached_pic.size))
		{
			_metadata.set_album_cover(txr, {150, 150});
			set_background(txr);
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

	// some formats/codecs have bad packet durations.
	// no idea what is causing this. as of right now mp3 has a workaround,
	// but anything else might end playback too early.
	switch (_adecoder->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_format.seek_file(-1, 1, 1, 1, AVSEEK_FLAG_FRAME);
		break;
	// ogg/opus is now broken, will not fix. just convert to mp3
	default:
		break;
	}
}

void audioviz::set_album_cover(const std::filesystem::path &image_path, const sf::Vector2f size)
{
	sf::Texture txr;
	if (!txr.loadFromFile(image_path))
		throw std::runtime_error("failed to load album cover: '" + image_path.string() + '\'');
	_metadata.set_album_cover(txr, size);
}

void audioviz::set_text_font(const std::filesystem::path &path)
{
	if (!font.loadFromFile(path))
		throw std::runtime_error("failed to load font: '" + path.string() + '\'');
	font_loaded = true;
}

void audioviz::metadata_init()
{
	auto &title_text = _metadata.title_text,
		 &artist_text = _metadata.artist_text;
	
	// set style, fontsize, and color
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);
	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});

	// set text using stream/format metadata
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
	ss.set_rect({{margin, margin}, {size.x - 2 * margin, size.y - 2 * margin}});
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
	spr.capture_centered_square_view();
	spr.fill_screen(size);

	bg.rt.orig.draw(spr);
	bg.rt.orig.display();
	bg.apply_fx();
}

void audioviz::draw_spectrum()
{
	ss.process(fa, sa, audio_buffer.data());
	spectrum.rt.orig.clear(zero_alpha);
	spectrum.rt.orig.draw(ss);
	spectrum.rt.orig.display();
	spectrum.apply_fx();
}

// should be called AFTER draw_spectrum()
void audioviz::draw_particles()
{
	ps.update(sa, size.y);
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
		pa_stream.emplace(0, 2, paFloat32, _astream.sample_rate(), fft_size);
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
	while ((int)audio_buffer.size() < 2 * fft_size)
	{
		const auto packet = _format.read_packet();

		if (!packet)
		{
			std::cerr << "packet is null; format probably reached eof\n";
			return;
		}

		// const auto &stream = _format.streams()[packet->stream_index];
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

bool audioviz::prepare_frame()
{
	decode_media();

	if (pa_stream)
		play_audio();

	// we don't have enough samples for fft; end here
	if ((int)audio_buffer.size() < 2 * fft_size)
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

	{ // brighten up bg with bass
		const auto &left_data = sa.left_data(),
			       &right_data = sa.right_data();
		assert(left_data.size() == right_data.size());

		const auto avg = (viz::util::weighted_max(left_data, [](auto x){return powf(x, 1 / 8.f);}) + viz::util::weighted_max(right_data, [](auto x){return powf(x, 1 / 8.f);})) / 2;

		dynamic_cast<fx::Mult &>(*bg.effects[2]).factor = 1 + avg;
		dynamic_cast<fx::Add &>(*bg.effects[3]).addend = 0.1 * avg;
		bg.apply_fx();
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

	target.draw(_metadata);
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
