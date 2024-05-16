#pragma once

#include <deque>
#include <list>
#include <optional>

#include <SFML/Graphics.hpp>
#include <av.hpp>
#include <portaudio.hpp>

#include "tt/Sprite.hpp"

#include "viz/ParticleSystem.hpp"
#include "viz/StereoSpectrum.hpp"

#include "fx/Blur.hpp"
#include "fx/Mult.hpp"
#include "fx/RenderTexture.hpp"

// TODO: add separate method for applying effects to the background

// will eventually create an `audioviz` namespace,
// move this class there and call it `stereo_spectrum`.
class audioviz : public sf::Drawable
{
public:
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;

protected:
	using SD = viz::SpectrumDrawable;
	using FS = tt::FrequencySpectrum;

private:
	static inline const sf::Color zero_alpha{0, 0, 0, 0};

	int sample_size = 3000;
	std::vector<float> audio_buffer;

	av::MediaReader _format;

	av::Stream _astream = _format.find_best_stream(AVMEDIA_TYPE_AUDIO);
	av::Decoder _adecoder = _astream.create_decoder();
	av::Resampler _resampler = av::Resampler(
		&_astream->codecpar->ch_layout, AV_SAMPLE_FMT_FLT, _astream.sample_rate(),
		&_astream->codecpar->ch_layout, (AVSampleFormat)_astream->codecpar->format, _astream.sample_rate());
	av::Frame rs_frame;

	std::optional<av::Stream> _vstream;
	std::optional<av::Decoder> _vdecoder;
	std::optional<av::Scaler> _scaler;
	std::optional<av::Frame> _scaled_frame;
	std::optional<std::list<sf::Texture>> _frame_queue;

	// framerate
	int framerate = 60;

	// audio frames per video frame
	int _afpvf = _astream.sample_rate() / framerate;

	// stereo spectrum!
	viz::StereoSpectrum ss = sample_size;

	// particle system
	viz::ParticleSystem ps;

	// metadata-related fields
	bool font_loaded = false;
	sf::Font font;
	sf::Text title_text = font,
			 artist_text = font;
	struct _ts
	{
		sf::Texture texture;
		tt::Sprite sprite;
		_ts() : sprite(texture) {}
	} album_cover;

	// clock to time the particle system
	sf::Clock ps_clock;

	std::optional<pa::PortAudio> pa_init;
	std::optional<pa::Stream> pa_stream;

public:
	class _fx
	{
		friend class audioviz;
		struct _rt_pair
		{
			fx::RenderTexture orig, with_fx;
			_rt_pair(const sf::Vector2u size, const int antialiasing)
				: orig(size, antialiasing), with_fx(size) {}
		} rt;

		_fx(const sf::Vector2u size, const int antialiasing)
			: rt(size, antialiasing) {}

	public:
		std::vector<std::unique_ptr<fx::Effect>> effects;

		// copies `orig` to `with_fx`, then applies available effects
		void apply_fx()
		{
			rt.with_fx.clear(zero_alpha);
			rt.with_fx.copy(rt.orig);
			for (const auto &effect : effects)
				rt.with_fx.apply(*effect);
		}
	} bg, particles, spectrum;

	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media_url url to media source. must contain an audio stream
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	audioviz(sf::Vector2u size, const std::string &media_url, int antialiasing = 4);

	/**
	 * @return whether the end of the audio buffer has been reached and another frame cannot be prepared.
	 */
	bool prepare_frame();

	/**
	 * Overrides `sf::Drawable::draw`.
	 */
	void draw(sf::RenderTarget &target, sf::RenderStates) const override;

	/**
	 * @return the chunk of audio used to produce the last frame
	 */
	const std::vector<float> &current_audio() const { return audio_buffer; }

	/// setters

	/**
	 * @param enabled whether to play the audio used to render the spectrum using PortAudio
	 */
	void set_audio_playback_enabled(bool enabled);

	/**
	 * important if you are capturing frames for video encoding!
	 */
	void set_framerate(int framerate);

	// set background image with optional effects: blur and color-multiply
	void set_background(const std::filesystem::path &image_path);
	void set_background(const sf::Texture &texture);

	// set margins around the output size for the spectrum to respect
	void set_margin(int margin);

	void set_title_text(const std::string &text);
	void set_artist_text(const std::string &text);
	void set_album_cover(const std::filesystem::path &image_path, sf::Vector2f size = {150, 150});

	// you **must** call this method in order to see text metadata!
	void set_text_font(const std::filesystem::path &path);

	// set the top-left-most position for the metadata (album cover sprite, title/artist text) to be drawn from
	void set_metadata_position(const sf::Vector2f pos);

	/// passthrough setters

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
	void av_init();
	void text_init();
	void draw_spectrum();
	void draw_particles();
	void _set_album_cover(sf::Vector2f size = {150, 150});
	void decode_media();
	void play_audio();
};