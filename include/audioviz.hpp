#pragma once

#include <list>
#include <optional>

#include <SFML/Graphics.hpp>

#include <av/Frame.hpp>
#include <av/MediaReader.hpp>
#include <av/Resampler.hpp>
#include <av/SwScaler.hpp>
#include <string>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#include "viz/Layer.hpp"
#include "viz/ParticleSystem.hpp"
#include "viz/SongMetadataDrawable.hpp"
#include "viz/StereoSpectrum.hpp"
#include "viz/VerticalBar.hpp"
#include "viz/VerticalPill.hpp"

class audioviz : public sf::Drawable
{
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;
	int ss_margin = 10;

	using BarType = viz::VerticalBar;
	using SD = viz::SpectrumDrawable<BarType>;
	using FS = tt::FrequencyAnalyzer;

	int fft_size = 3000;
	std::vector<float> audio_buffer;

	// TODO: test dynamic media file changes
	struct _media
	{
		const std::string url;
		av::MediaReader _format;

		av::Stream _astream = _format.find_best_stream(AVMEDIA_TYPE_AUDIO);
		av::Decoder _adecoder = _astream.create_decoder();
		av::Resampler _resampler = av::Resampler(
			{&_astream->codecpar->ch_layout, AV_SAMPLE_FMT_FLT, _astream.sample_rate()},						   // output params
			{&_astream->codecpar->ch_layout, (AVSampleFormat)_astream->codecpar->format, _astream.sample_rate()}); // input params
		av::Frame rs_frame;

		std::optional<sf::Texture> attached_pic;

		std::optional<av::Stream> _vstream;
		std::optional<av::Decoder> _vdecoder;
		std::optional<av::SwScaler> _scaler;
		std::optional<av::Frame> _scaled_frame;
		std::optional<std::list<sf::Texture>> _frame_queue;

		_media(const std::string &url)
			: url(url),
			  _format(url)
		{
		}

		void init(const audioviz &);
		void decode(audioviz &);
	};
	friend _media;

	// MUST be constructed in the constructor
	std::optional<_media> media;

	// framerate
	int framerate = 60;

	// audio frames per video frame
	int _afpvf = media->_astream.sample_rate() / framerate;

	// fft processor
	tt::FrequencyAnalyzer fa = fft_size;
	tt::StereoAnalyzer sa;

	// stereo spectrum
	viz::StereoSpectrum<BarType> ss;
	std::optional<sf::BlendMode> spectrum_bm;

	// particle system
	viz::ParticleSystem<sf::CircleShape> ps;

	// metadata-related fields
	bool font_loaded = false;
	sf::Font font;
	viz::SongMetadataDrawable _metadata = font;

	// clocks for timing stuff
	sf::Clock ps_clock;

	// timing text
	sf::Text timing_text;
	std::ostringstream tt_ss;
	bool tt_enabled = false;

#ifdef AUDIOVIZ_PORTAUDIO
	// PortAudio stuff for live playback
	std::optional<pa::PortAudio> pa_init;
	std::optional<pa::Stream> pa_stream;
#endif

	std::vector<viz::Layer> layers;
	tt::RenderTexture final_rt;

public:
	// viz::Layer bg, particles, spectrum;
	void use_attached_pic_as_bg();

	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media_url url to media source. must contain an audio stream
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	audioviz(sf::Vector2u size, const std::string &media_url, int antialiasing = 4);

	/**
	 * Add default effects to the `bg`, `spectrum`, and `particles` layers.
	 */
	void add_default_effects();

	/**
	 * Prepare a frame to be drawn with `draw()`.
	 * @return Whether another frame can be prepared
	 */
	bool prepare_frame();

	void draw(sf::RenderTarget &target, sf::RenderStates) const override;

	viz::Layer &add_layer(const std::string &name, int antialiasing);
	viz::Layer *get_layer(const std::string &name);

	/// setters

#ifdef AUDIOVIZ_PORTAUDIO
	/**
	 * @param enabled When started with `start()`, whether to play the audio used to render the spectrum
	 */
	void set_audio_playback_enabled(bool enabled);
#endif

	void set_timing_text_enabled(bool enabled);

	sf::Vector2u get_size() const { return size; }

	/* resizable windows not happening now, too much of the codebase relies on a static window size
	void set_size(sf::Vector2u size);
	*/

	// important if you are capturing frames for video encoding!
	int get_framerate() const { return framerate; }
	void set_framerate(int framerate);

	// set background image with optional effects: blur and color-multiply
	// void set_background(const std::string &image_path);
	void set_background(const sf::Texture &texture);

	// set margins around the output size for the spectrum to respect
	void set_spectrum_margin(int margin);

	// set blend mode for spectrum against target
	void set_spectrum_blendmode(const sf::BlendMode &);

	void set_album_cover(const std::string &image_path, sf::Vector2f size = {150, 150});

	// you **must** call this method in order to see text metadata!
	void set_text_font(const std::string &path);

	void set_media_url(const std::string &url);
	const std::string &get_media_url() const;

	/// passthrough setters
	/// TODO: using viz::Layer, use dependency injection to get rid of these setters!!!!!
	///       give the caller the power to customize their visualizer further!!!!!!!!!!!!!

	void set_sample_size(int);
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

private:
	void metadata_init();
	void draw_spectrum();
	void draw_particles();
	void play_audio();
	void capture_elapsed_time(const char *const label, const sf::Clock &_clock);
};
