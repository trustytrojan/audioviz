#pragma once

#include <SFML/Graphics.hpp>
// #include <sndfile.hh>
#include "AudioDecoder.hpp"
#include "SpectrumDrawable.hpp"
#include "MyRenderTexture.hpp"
#include "PortAudio.hpp"
#include "ParticleSystem.hpp"

class audioviz
{
private:
	using SD = SpectrumDrawable;
	using FS = FrequencySpectrum;

	const sf::Vector2u size;
	int sample_size = 3000;
	const std::string audio_file;

	AudioDecoder ad = audio_file;

	int framerate = 60,
		afpvf = ad.sample_rate() / framerate;
	// afpvf = sf.samplerate() / framerate;

	// SndfileHandle sf;
	std::vector<float> audio_buffer =
		// std::vector<float>(sample_size * sf.channels());
		std::vector<float>(sample_size * ad.nb_channels());
	std::vector<float> full_audio = std::vector<float>(ad.frames() * ad.nb_channels());
	int full_audio_idx = 0;
	SD sd = sample_size;
	ParticleSystem ps;

	sf::Font font;
	sf::Text title_text, artist_text;

	// TODO: bring the particles to life

	struct _texture_sprite
	{
		sf::Texture texture;
		sf::Sprite sprite;
		_texture_sprite() : sprite(texture) {}
	} album_cover;

	struct _rt
	{
		struct _rt_blur
		{
			MyRenderTexture original, blurred;
			_rt_blur(sf::Vector2u size, int antialiasing)
				: original(size, sf::ContextSettings(0, 0, antialiasing)),
				  blurred(size) {}
		} spectrum, particles;

		MyRenderTexture bg;

		_rt(sf::Vector2u size, int antialiasing)
			: spectrum(size, antialiasing),
			  particles(size, antialiasing),
			  bg(size, sf::ContextSettings(0, 0, antialiasing)) {}
	} rt;

public:
	audioviz(sf::Vector2u size, const std::string &audio_file, int antialiasing = 4);
	void set_framerate(int framerate);
	void set_bg(const std::string &file);
	Pa::Stream<float> create_pa_stream();
	bool draw_frame(sf::RenderTarget &target, Pa::Stream<float> *const pa_stream = NULL);

	// passthrough setters
	void set_bar_width(int width);
	void set_bar_spacing(int spacing);
	void set_color_mode(SD::ColorMode mode);
	void set_solid_color(sf::Color color);
	void set_color_wheel_rate(float rate);
	void set_color_wheel_hsv(sf::Vector3f hsv);
	void set_multiplier(float multiplier);
	void set_fft_size(int fft_size);
	void set_interp_type(FS::InterpolationType interp_type);
	void set_scale(FS::Scale scale);
	void set_nth_root(int nth_root);
	void set_accum_method(FS::AccumulationMethod method);
	void set_window_func(FS::WindowFunction wf);
};