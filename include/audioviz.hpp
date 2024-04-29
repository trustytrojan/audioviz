#pragma once

#include <future>
#include <optional>

#include <SFML/Graphics.hpp>

#include "AudioDecoder.hpp"
#include "SpectrumDrawable.hpp"
#include "MyRenderTexture.hpp"
#include "ParticleSystem.hpp"
#include "InterleavedAudioBuffer.hpp"

#include "pa/PortAudio.hpp"
#include "pa/Stream.hpp"

// will eventually create an `audioviz` namespace,
// move this class there and call it `stereo_spectrum`.
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
	SpectrumDrawable
		sd_left = sample_size,
		sd_right = sample_size;

	// particle system
	ParticleSystem ps;

	// metadata-related fields
	sf::Font font;
	sf::Text title_text, artist_text;
	struct _texture_sprite
	{
		sf::Texture texture;
		sf::Sprite sprite;
		_texture_sprite() : sprite(texture) {}
	} album_cover;

	// container to hold render-textures
	struct _rt
	{
		struct _blur
		{
			MyRenderTexture original, blurred;
			_blur(const sf::Vector2u size, const int antialiasing);
		} spectrum, particles;

		MyRenderTexture bg;
		_rt(const sf::Vector2u size, const int antialiasing);
	} rt;

	// clock to time the particle system
	sf::Clock ps_clock;

	std::optional<pa::PortAudio> pa_init;
	std::optional<pa::Stream<float>> pa_stream;

public:
	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media_url url to media source. must contain an audio stream
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	audioviz(sf::Vector2u size, const std::string &media_url, int antialiasing = 4);

	/**
	 * @return whether the end of the audio buffer has been reached and another frame cannot be produced.
	 */
	bool draw_frame(sf::RenderTarget &target);

	/**
	 * @return the chunk of audio used to produce the last frame
	 */
	const std::span<float> &current_audio() const;

	void set_audio_playback_enabled(bool enabled);

	// setters

	/**
	 * important if you are capturing frames for video encoding!
	 * the framerate defaults to 60fps, so if you are on a high-refresh rate display,
	 * it may be setting a higher framerate.
	 */
	void set_framerate(int framerate);

	// set background image with optional effects: blur and color-multiply
	void set_background(const std::filesystem::path &image_path, EffectOptions options = {{10, 10, 25}, 0});

	// set margins around the output size for the spectrum to respect
	void set_margin(int margin);

	void set_title_text(const std::string &text);
	void set_artist_text(const std::string &text);
	void set_album_cover(const std::filesystem::path &image_path, sf::Vector2f scale_to = {150, 150});

	// you **must** call this method in order to see text metadata!
	void set_text_font(const std::filesystem::path &path);

	// set the top-left-most position for the metadata (album cover sprite, title/artist text) to be drawn from
	void set_metadata_position(const sf::Vector2f &pos);

	// passthrough setters

	void set_bar_width(int width);
	void set_bar_spacing(int spacing);
	void set_color_mode(SD::ColorMode mode);
	void set_solid_color(sf::Color color);
	void set_color_wheel_rate(float rate);
	void set_color_wheel_hsv(sf::Vector3f hsv);
	void set_multiplier(float multiplier);
	// void set_fft_size(int fft_size); // will handle this later
	void set_interp_type(FS::InterpolationType interp_type);
	void set_scale(FS::Scale scale);
	void set_nth_root(int nth_root);
	void set_accum_method(FS::AccumulationMethod method);
	void set_window_func(FS::WindowFunction wf);

private:
	void play_audio();
	void decoder_thread_func();
	bool decoder_thread_finished();
	void set_text_defaults();
	void draw_spectrum();
	void draw_particles();
	void blur_spectrum();
	void blur_particles();
	void actually_draw_on_target(sf::RenderTarget &target);
};