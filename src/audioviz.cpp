#include "audioviz.hpp"
#include <numeric>

audioviz::audioviz(sf::Vector2u size, std::string audio_file, int antialiasing)
	: size(size),
	  audio_file(audio_file),
	  ps(size, 50),
	  rt(size, antialiasing)
{
	if (sf.channels() != 2)
		throw std::runtime_error("only stereo audio is supported!");
	if (!blur_shader.loadFromFile("blur.frag", sf::Shader::Type::Fragment))
		throw std::runtime_error("failed to load blur shader: 'blur.frag' required in current directory");
}

void audioviz::set_framerate(int framerate)
{
	this->framerate = framerate;
	afpvf = sf.samplerate() / framerate;
}

void audioviz::set_bg(const std::string &file)
{
	if (!bg.texture.loadFromFile(file))
		throw std::runtime_error("failed to load background image: '" + file + '\'');

	// sprites do not receive texture changes, so need to reset the texture rect
	bg.sprite.setTexture(bg.texture, true);

	// make sure bgTexture fills up the whole screen, and is centered
	const auto tsize = bg.texture.getSize();
	bg.sprite.setOrigin({tsize.x / 2, tsize.y / 2});
	bg.sprite.setPosition({size.x / 2, size.y / 2});
	const auto scale = std::max((float)size.x / tsize.x, (float)size.y / tsize.y);
	bg.sprite.setScale({scale, scale});
}

PortAudio::Stream audioviz::create_pa_stream()
{
	return PortAudio::Stream(0, sf.channels(), paFloat32, sf.samplerate(), sample_size);
}

static void debug_rects(sf::RenderTarget &target, const sf::IntRect &left_half, const sf::IntRect &right_half)
{
	sf::RectangleShape r1((sf::Vector2f)left_half.getSize()), r2((sf::Vector2f)right_half.getSize());
	r1.setPosition((sf::Vector2f)left_half.getPosition());
	r2.setPosition((sf::Vector2f)right_half.getPosition());
	r1.setFillColor(sf::Color{0});
	r2.setFillColor(sf::Color{0});
	r1.setOutlineThickness(1);
	r2.setOutlineThickness(1);
	target.draw(r1);
	target.draw(r2);
}

bool audioviz::draw_frame(sf::RenderTarget &target, PortAudio::Stream *const pa_stream)
{
	static const sf::Color zero_alpha(0);

	// TODO: get rid of this, maybe add a rect parameter????
	// that would make this even more modular!!!!!!!!!!!
	static const auto margin = 15;

	if (target.getSize() != size)
		throw std::runtime_error("target size must match render-texture size!");

	// read audio from file
	const auto frames_read = sf.readf(audio_buffer.data(), sample_size);

	// no audio left, we are done
	if (!frames_read)
		return false;

	try // to play the audio
	{
		if (pa_stream)
			pa_stream->write(audio_buffer.data(), afpvf);
	}
	catch (const PortAudio::Error &e)
	{
		if (e.err != paOutputUnderflowed)
			throw;
		std::cerr << "output underflowed\n";
	}

	// not enough audio to perform fft, we are done
	if (frames_read != sample_size)
		return false;

	// stereo rectangles to draw to
	const sf::IntRect
		left_half{
			{margin, margin},
			{(size.x - (2 * margin)) / 2.f - (sd.bar.get_spacing() / 2.f), size.y - (2 * margin)}},
		right_half{
			{(size.x / 2.f) + (sd.bar.get_spacing() / 2.f), margin},
			{(size.x - (2 * margin)) / 2.f - (sd.bar.get_spacing() / 2.f), size.y - (2 * margin)}};

	rt.original.clear(zero_alpha);
	{ // draw spectrum on rt.original
		const auto dist_between_rects = right_half.left - (left_half.left + left_half.width);
		assert(dist_between_rects == sd.bar.get_spacing());

		sd.copy_channel_to_input(audio_buffer.data(), 2, 0, true);
		sd.draw(rt.original, left_half, true);

		const auto &spectrum = sd.get_spectrum();
		const auto amount_to_avg = spectrum.size() / 4.f;
		float speeds[2];
		speeds[0] = std::accumulate(spectrum.begin(), spectrum.begin() + amount_to_avg, 0.f) / amount_to_avg;

		sd.copy_channel_to_input(audio_buffer.data(), 2, 1, true);
		sd.draw(rt.original, right_half);
		speeds[1] = std::accumulate(spectrum.begin(), spectrum.begin() + amount_to_avg, 0.f) / amount_to_avg;

		const auto speeds_avg = (speeds[0] + speeds[1]) / 2;

		rt.particles.clear(zero_alpha);
		const auto speed_increase = sqrtf(size.y * speeds_avg);
		ps.draw(rt.particles, {0, -speed_increase});
		rt.particles.display();
	}
	rt.original.display();

	// copy it to another render-texture for blurring
	rt.blurred.clear(zero_alpha);
	rt.blurred.draw(rt.original.sprite);
	rt.blurred.display();

	// blur the spectrum
	rt.blurred.blur(blur_shader, 1, 1, 20);

	// layer everything together
	target.draw(bg.sprite);
	target.draw(rt.particles.sprite, sf::BlendAdd);
	target.draw(rt.blurred.sprite, sf::BlendAdd);

	// redraw original spectrum over everything else
	// need to do this because anti-aliased edges will copy the
	// background they are drawn on, completely ignoring alpha values.
	// i really need to separate the actions SpectrumDrawable::draw takes.
	sd.copy_channel_to_input(audio_buffer.data(), 2, 0, true);
	sd.draw(target, left_half, true);
	sd.copy_channel_to_input(audio_buffer.data(), 2, 1, true);
	sd.draw(target, right_half);

	// seek audio backwards
	sf.seek(afpvf - sample_size, SEEK_CUR);
	return true;
}

void audioviz::set_bar_width(int width)
{
	sd.bar.set_width(width);
}

void audioviz::set_bar_spacing(int spacing)
{
	sd.bar.set_spacing(spacing);
}

void audioviz::set_color_mode(SD::ColorMode mode)
{
	sd.color.set_mode(mode);
}

void audioviz::set_solid_color(sf::Color color)
{
	sd.color.set_solid_rgb(color);
}

void audioviz::set_color_wheel_rate(float rate)
{
	sd.color.wheel.set_rate(rate);
}

void audioviz::set_color_wheel_hsv(sf::Vector3f hsv)
{
	sd.color.wheel.set_hsv(hsv);
}

void audioviz::set_multiplier(float multiplier)
{
	sd.set_multiplier(multiplier);
}

void audioviz::set_fft_size(int fft_size)
{
	sd.set_fft_size(fft_size);
}

void audioviz::set_interp_type(FS::InterpolationType interp_type)
{
	sd.set_interp_type(interp_type);
}

void audioviz::set_scale(FS::Scale scale)
{
	sd.set_scale(scale);
}

void audioviz::set_nth_root(int nth_root)
{
	sd.set_nth_root(nth_root);
}

void audioviz::set_accum_method(FS::AccumulationMethod method)
{
	sd.set_accum_method(method);
}

void audioviz::set_window_func(FS::WindowFunction wf)
{
	sd.set_window_func(wf);
}