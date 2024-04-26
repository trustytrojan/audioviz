#include "audioviz.hpp"
#include <numeric>
#include <mutex>

static std::mutex mtx;

audioviz::audioviz(sf::Vector2u size, const std::string &audio_file, int antialiasing)
	: size(size),
	  audio_file(audio_file),
	  ab(ad.nb_channels()),
	  ps(size, 50),
	  title_text(font, ad.get_metadata_entry("title")),
	  artist_text(font, ad.get_metadata_entry("artist")),
	  rt(size, antialiasing)
{
	setup_metadata_sprites();
	if (ad.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");
	set_margin(10);
}

void audioviz::decoder_thread_func(audioviz *const av)
{
	pthread_setname_np(pthread_self(), "DecoderThread");
	std::vector<float> tmpbuf;
	while (1)
	{
		if (!(av->ad >> tmpbuf))
			break;
		mtx.lock();
		av->ab.buffer().insert(av->ab.buffer().end(), tmpbuf.begin(), tmpbuf.end());
		mtx.unlock();
	}
	std::cout << "decoder thread finished\n";
}

bool audioviz::decoder_thread_finished()
{
	return decoder_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void audioviz::setup_metadata_sprites()
{
	if (!font.loadFromFile("/usr/share/fonts/TTF/Iosevka-Regular.ttc"))
		throw std::runtime_error("failed to load font!");
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);

	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);

	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});

	if (!album_cover.texture.loadFromFile("images/midnight.jpg"))
		throw std::runtime_error("failed to load album cover!");
	album_cover.sprite.setTexture(album_cover.texture, true);

	const sf::Vector2f ac_pos{30, 30};
	album_cover.sprite.setPosition(ac_pos);

	// using album_cover.texture.getSize(), set the scale on album_cover.sprite
	// such that it will only take up a 50x50 area
	const sf::Vector2f ac_size{150, 150};
	const auto ac_tsize = album_cover.texture.getSize();
	album_cover.sprite.setScale({ac_size.x / ac_tsize.x, ac_size.y / ac_tsize.y});

	const sf::Vector2f metadata_pos{ac_pos.x + ac_size.x + 10, ac_pos.y};
	title_text.setPosition({metadata_pos.x, metadata_pos.y});
	artist_text.setPosition({metadata_pos.x, metadata_pos.y + title_text.getCharacterSize() + 5});
}

void audioviz::set_framerate(int framerate)
{
	this->framerate = framerate;
	// afpvf = sf.samplerate() / framerate;
	afpvf = ad.sample_rate() / framerate;
}

void audioviz::set_bg(const std::string &file)
{
	sf::Texture bg_texture;

	if (!bg_texture.loadFromFile(file))
		throw std::runtime_error("failed to load background image: '" + file + '\'');

	sf::Sprite bg_sprite(bg_texture);

	// make sure bgTexture fills up the whole screen, and is centered
	const auto tsize = bg_texture.getSize();
	bg_sprite.setOrigin({tsize.x / 2, tsize.y / 2});
	bg_sprite.setPosition({size.x / 2, size.y / 2});
	const auto scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
	bg_sprite.setScale({scale, scale});

	rt.bg.draw(bg_sprite);
	rt.bg.blur(10, 10, 25);
	// rt.bg.mult(0.5);
}

Pa::Stream<float> audioviz::create_pa_stream()
{
	return Pa::Stream<float>(0, ad.nb_channels(), ad.sample_rate(), sample_size);
}

void audioviz::set_margin(const int margin)
{
	const sf::IntRect
		left_half{
			{margin, margin},
			{(size.x - (2 * margin)) / 2.f - (sd_left.bar.get_spacing() / 2.f), size.y - (2 * margin)}},
		right_half{
			{(size.x / 2.f) + (sd_right.bar.get_spacing() / 2.f), margin},
			{(size.x - (2 * margin)) / 2.f - (sd_right.bar.get_spacing() / 2.f), size.y - (2 * margin)}};

	const auto dist_between_rects = right_half.left - (left_half.left + left_half.width);
	assert(dist_between_rects == sd_left.bar.get_spacing());

	sd_left.set_target_rect(left_half, true);
	sd_right.set_target_rect(right_half);

	assert(sd_left.get_spectrum_data().size() == sd_right.get_spectrum_data().size());
}

void audioviz::play_audio(Pa::Stream<float> &pa_stream)
{
	try // to play the audio
	{
		pa_stream.write(audio_buffer.data(), afpvf);
	}
	catch (const Pa::Error &e)
	{
		if (e.code != paOutputUnderflowed)
			throw;
		std::cerr << e.what() << '\n';
	}
}

void audioviz::draw_spectrum()
{
	sd_left.do_fft(audio_buffer.data(), 2, 0, true);
	sd_right.do_fft(audio_buffer.data(), 2, 1, true);

	rt.spectrum.original.clear(zero_alpha);
	rt.spectrum.original.draw(sd_left);
	rt.spectrum.original.draw(sd_right);
	rt.spectrum.original.display();
}

void audioviz::draw_particles()
{
	const auto sd_left_data = sd_left.get_spectrum_data(),
			   sd_right_data = sd_right.get_spectrum_data();
	assert(sd_left_data.size() == sd_right_data.size());

	float speeds[2];
	const auto amount_to_avg = sd_left_data.size() / 4.f;

	speeds[0] = std::accumulate(sd_left_data.begin(), sd_left_data.begin() + amount_to_avg, 0.f) / amount_to_avg;
	speeds[1] = std::accumulate(sd_right_data.begin(), sd_right_data.begin() + amount_to_avg, 0.f) / amount_to_avg;

	const auto speeds_avg = (speeds[0] + speeds[1]) / 2;

	rt.particles.original.clear(zero_alpha);
	const auto speed_increase = powf(size.y * speeds_avg, 1.f / 2.6666667f);
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

	// new discovery...........................
	// this is how to invert the color??????
	// sf::BlendMode spectrum_blend(
	// 	sf::BlendMode::Factor::SrcAlpha,
	// 	sf::BlendMode::Factor::DstAlpha,
	// 	sf::BlendMode::Equation::ReverseSubtract);
	// target.draw(rt.spectrum.blurred.sprite(), spectrum_blend);

	target.draw(rt.spectrum.blurred.sprite(), sf::BlendAdd);

	// redraw spectrum due to anti-aliased edges retaining original background color
	target.draw(sd_left);
	target.draw(sd_right);
	// target.draw(rt.spectrum.original.sprite());

	// draw metadata
	target.draw(album_cover.sprite);
	target.draw(title_text, sf::BlendAlpha);
	target.draw(artist_text, sf::BlendAlpha);
}

bool audioviz::draw_frame(sf::RenderTarget &target, Pa::Stream<float> *const pa_stream)
{
	mtx.lock();
	int frames_read = ab.read_frames(audio_buffer, sample_size);
	while (!decoder_thread_finished() && frames_read < sample_size)
		frames_read += ab.read_frames(audio_buffer, sample_size);
	mtx.unlock();

	if (pa_stream)
		play_audio(*pa_stream);
	if (frames_read != sample_size)
		return false;

	draw_spectrum();
	draw_particles();
	blur_spectrum();
	blur_particles();

	actually_draw_on_target(target);

	// seek audio backwards
	ab.seek(afpvf - sample_size, SEEK_CUR);

	return true;
}

void audioviz::set_bar_width(int width)
{
	sd_left.bar.set_width(width);
	sd_right.bar.set_width(width);
}

void audioviz::set_bar_spacing(int spacing)
{
	sd_left.bar.set_spacing(spacing);
	sd_right.bar.set_spacing(spacing);
}

void audioviz::set_color_mode(SD::ColorMode mode)
{
	sd_left.color.set_mode(mode);
	sd_right.color.set_mode(mode);
}

void audioviz::set_solid_color(sf::Color color)
{
	sd_left.color.set_solid_rgb(color);
	sd_right.color.set_solid_rgb(color);
}

void audioviz::set_color_wheel_rate(float rate)
{
	sd_left.color.wheel.set_rate(rate);
	sd_right.color.wheel.set_rate(rate);
}

void audioviz::set_color_wheel_hsv(sf::Vector3f hsv)
{
	sd_left.color.wheel.set_hsv(hsv);
	sd_right.color.wheel.set_hsv(hsv);
}

void audioviz::set_multiplier(float multiplier)
{
	sd_left.set_multiplier(multiplier);
	sd_right.set_multiplier(multiplier);
}

// void audioviz::set_fft_size(int fft_size)
// {
// 	sd_left.set_fft_size(fft_size);
// 	sd_right.set_fft_size(fft_size);
// }

void audioviz::set_interp_type(FS::InterpolationType interp_type)
{
	sd_left.set_interp_type(interp_type);
	sd_right.set_interp_type(interp_type);
}

void audioviz::set_scale(FS::Scale scale)
{
	sd_left.set_scale(scale);
	sd_right.set_scale(scale);
}

void audioviz::set_nth_root(int nth_root)
{
	sd_left.set_nth_root(nth_root);
	sd_right.set_nth_root(nth_root);
}

void audioviz::set_accum_method(FS::AccumulationMethod method)
{
	sd_left.set_accum_method(method);
	sd_right.set_accum_method(method);
}

void audioviz::set_window_func(FS::WindowFunction wf)
{
	sd_left.set_window_func(wf);
	sd_right.set_window_func(wf);
}