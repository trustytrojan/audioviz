#pragma once

#include <SFML/Graphics.hpp>
#include <sndfile.hh>
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
	std::string audio_file;
	int framerate = 60, afpvf = sf.samplerate() / framerate;

	SndfileHandle sf = audio_file;
	std::vector<float> audio_buffer = std::vector<float>(sample_size * sf.channels());
	SD sd = sample_size;
	sf::Shader blur_shader;
	ParticleSystem ps;

	// TODO: bring the particles to life

	struct _bg
	{
		sf::Texture texture;
		sf::Sprite sprite;
		_bg() : sprite(texture) {}
	} bg;

	struct _spectrum_rt
	{
		MyRenderTexture original, blurred;
		_spectrum_rt(sf::Vector2u size, int antialiasing)
			: original(size, sf::ContextSettings(0, 0, antialiasing)),
			  blurred(size) {}
	} spectrum_rt;

public:
	audioviz(sf::Vector2u size, std::string audio_file, int antialiasing = 4);
	void set_framerate(int framerate);
	void set_bg(const std::string &file);
	PortAudio::Stream create_pa_stream();
	bool draw_frame(sf::RenderTarget &target, PortAudio::Stream *const pa_stream = NULL);

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