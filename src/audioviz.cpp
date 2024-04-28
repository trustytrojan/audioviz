#include "audioviz.hpp"
#include <numeric>
#include <mutex>

audioviz::audioviz(const sf::Vector2u size, const std::string &audio_file, const int antialiasing)
	: size(size),
	  ad(audio_file),
	  ps(size, 50),
	  title_text(font, ad.get_metadata_entry("title")),
	  artist_text(font, ad.get_metadata_entry("artist")),
	  rt(size, antialiasing)
{
	set_text_defaults();

	// for now only stereo is supported
	if (ad.nb_channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");

	// default margin around the target
	set_margin(10);

	// default metadata position
	set_metadata_position({30, 30});
}

const std::span<float> &audioviz::current_audio() const
{
	return audio_span;
}

void audioviz::decoder_thread_func()
{
	pthread_setname_np(pthread_self(), "DecoderThread");
	while (ad.append_to(ab.buffer()))
		;
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

void audioviz::set_album_cover(const std::filesystem::path &image_path, const sf::Vector2f desired_size)
{
	if (!album_cover.texture.loadFromFile(image_path))
		throw std::runtime_error("failed to load album cover: '" + image_path.string() + '\'');
	album_cover.sprite.setTexture(album_cover.texture, true);

	// using `album_cover.texture`'s size, set the scale on `album_cover.sprite`
	// such that it will only take up a certain area
	// widescreen images will end up looking small but it's fine
	const auto ac_tsize = album_cover.texture.getSize();
	float scale_factor = std::min(desired_size.x / ac_tsize.x, desired_size.y / ac_tsize.y);
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
	title_text.setStyle(sf::Text::Bold | sf::Text::Italic);
	artist_text.setStyle(sf::Text::Italic);
	title_text.setCharacterSize(24);
	artist_text.setCharacterSize(24);
	title_text.setFillColor({255, 255, 255, 150});
	artist_text.setFillColor({255, 255, 255, 150});
}

void audioviz::set_framerate(int framerate)
{
	this->framerate = framerate;
	// afpvf = sf.samplerate() / framerate;
	afpvf = ad.sample_rate() / framerate;
}

void audioviz::set_background(const std::filesystem::path &image_path, const EffectOptions opts)
{
	sf::Texture bg_texture;
	if (!bg_texture.loadFromFile(image_path))
		throw std::runtime_error("failed to load background image: '" + image_path.string() + '\'');
	sf::Sprite bg_sprite(bg_texture);

	// make sure bgTexture fills up the whole screen, and is centered
	const auto tsize = bg_texture.getSize();
	bg_sprite.setOrigin({tsize.x / 2, tsize.y / 2});
	bg_sprite.setPosition({size.x / 2, size.y / 2});
	const auto scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
	bg_sprite.setScale({scale, scale});

	rt.bg.draw(bg_sprite);

	if (opts.blur.hrad && opts.blur.vrad && opts.blur.n_passes)
		rt.bg.blur(opts.blur.hrad, opts.blur.vrad, opts.blur.n_passes);
	if (opts.mult)
		rt.bg.mult(opts.mult);
}

Pa::Stream<float> audioviz::create_pa_stream() const
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

	assert(sd_left.data().size() == sd_right.data().size());
}

void audioviz::play_audio(Pa::Stream<float> &pa_stream)
{
	try // to play the audio
	{
		pa_stream.write(audio_span.data(), afpvf);
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
	sd_left.do_fft(audio_span.data(), 2, 0, true);
	sd_right.do_fft(audio_span.data(), 2, 1, true);
	rt.spectrum.original.clear(zero_alpha);
	rt.spectrum.original.draw(sd_left);
	rt.spectrum.original.draw(sd_right);
	rt.spectrum.original.display();
}

// should be called AFTER draw_spectrum()
void audioviz::draw_particles()
{
	const auto left_data = sd_left.data(),
			   right_data = sd_right.data();
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
	target.draw(sd_left);
	target.draw(sd_right);

	// draw metadata
	if (album_cover.texture.getNativeHandle())
		target.draw(album_cover.sprite);
	target.draw(title_text);
	target.draw(artist_text);
}

bool audioviz::draw_frame(sf::RenderTarget &target, Pa::Stream<float> *const pa_stream)
{
	// wait for enough audio to be decoded
	// or, if decoding finished, just use what we can get
	while (!decoder_thread_finished() && (int)ab.frames_available() < sample_size)
		;

	// get frames using a span to avoid a copy
	const int frames_read = ab.read_frames(audio_span, sample_size);

	// play audio if necessary (synchronously, to stay in sync with the frame)
	if (pa_stream)
		play_audio(*pa_stream);

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