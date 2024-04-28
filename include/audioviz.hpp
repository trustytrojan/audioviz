#pragma once

#include <SFML/Graphics.hpp>
#include "AudioDecoder.hpp"
#include "SpectrumDrawable.hpp"
#include "MyRenderTexture.hpp"
#include "PortAudio.hpp"
#include "ParticleSystem.hpp"
#include "InterleavedAudioBuffer.hpp"
#include <future>

class audioviz
{
private:
	using SD = SpectrumDrawable;
	using FS = FrequencySpectrum;

	static inline const sf::Color zero_alpha{0, 0, 0, 0};

	struct EffectOptions
	{
		struct
		{
			float hrad = 10, vrad = 10;
			int n_passes = 25;
		} blur;
		float mult = 0;
	};

	const sf::Vector2u size;
	int sample_size = 3000;

	AudioDecoder ad;
	InterleavedAudioBuffer ab = ad.nb_channels();

	// this is a view of the decoded audio in `ab`
	std::span<float> audio_span;

	// offload audio decoding to a separate thread using `std::async`
	std::future<void> decoder_future = std::async(std::launch::async, &audioviz::decoder_thread_func, this);

	// framerate
	int framerate = 60;

	// audio frames per video frame
	int afpvf = ad.sample_rate() / framerate;

	// left and right spectrums
	SD sd_left = sample_size, sd_right = sample_size;

	// particle system
	ParticleSystem ps;

	// metadata font
	sf::Font font;

	// metadata text
	sf::Text title_text, artist_text;

	// album cover
	struct _texture_sprite
	{
		sf::Texture texture;
		sf::Sprite sprite;
		_texture_sprite() : sprite(texture) {}
	} album_cover;

	// render textures
	struct _rt
	{
		struct _rt_blur
		{
			MyRenderTexture original, blurred;
			_rt_blur(const sf::Vector2u size, const int antialiasing)
				: original(size, sf::ContextSettings(0, 0, antialiasing)),
				  blurred(size) {}
		} spectrum, particles;

		MyRenderTexture bg;

		_rt(const sf::Vector2u size, const int antialiasing)
			: spectrum(size, antialiasing),
			  particles(size, antialiasing),
			  bg(size, sf::ContextSettings(0, 0, antialiasing)) {}
	} rt;

public:
	audioviz(sf::Vector2u size, const std::string &audio_file, int antialiasing = 4);

	Pa::Stream<float> create_pa_stream() const;
	bool draw_frame(sf::RenderTarget &target, Pa::Stream<float> *const pa_stream = NULL);
	const std::span<float> &current_audio() const;

	// decoder thread functions

	void decoder_thread_func();
	bool decoder_thread_finished();

	// setters

	void set_framerate(int framerate);
	void set_background(const std::filesystem::path &image_path, EffectOptions options = {{10, 10, 25}, 0});
	void set_margin(int margin);
	void set_title_text(const std::string &text);
	void set_artist_text(const std::string &text);
	void set_album_cover(const std::filesystem::path &image_path, sf::Vector2f scale_to = {150, 150});
	void set_text_font(const std::filesystem::path &path);
	void set_metadata_position(const sf::Vector2f &pos);

	// passthrough setters

	void set_bar_width(int width);
	void set_bar_spacing(int spacing);
	void set_color_mode(SD::ColorMode mode);
	void set_solid_color(sf::Color color);
	void set_color_wheel_rate(float rate);
	void set_color_wheel_hsv(sf::Vector3f hsv);
	void set_multiplier(float multiplier);
	// void set_fft_size(int fft_size);
	void set_interp_type(FS::InterpolationType interp_type);
	void set_scale(FS::Scale scale);
	void set_nth_root(int nth_root);
	void set_accum_method(FS::AccumulationMethod method);
	void set_window_func(FS::WindowFunction wf);

private:
	void set_text_defaults();
	void play_audio(Pa::Stream<float> &pa_stream);
	void draw_spectrum();
	void draw_particles();
	void blur_spectrum();
	void blur_particles();
	void actually_draw_on_target(sf::RenderTarget &target);
};