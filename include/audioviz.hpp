#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#include "viz/Layer.hpp"
#include "viz/ParticleSystem.hpp"
#include "viz/SongMetadataDrawable.hpp"
#include "viz/StereoSpectrum.hpp"
#include "viz/VerticalBar.hpp"
#include "viz/ScopeDrawable.hpp"

#include "media/Media.hpp"

class audioviz : public sf::Drawable
{
public:
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;

private:
	using BarType = viz::VerticalBar;
	using ParticleShapeType = sf::CircleShape;

	int fft_size{3000};
	int framerate{60};

	// used for updating the particle system at 60Hz rate when framerate > 60
	int frame_count{}, vfcount{1};

	std::unique_ptr<Media> media;

	// audio frames per video frame
	int afpvf{media->astream().sample_rate() / framerate};

	// fft processor
	tt::FrequencyAnalyzer &fa;
	tt::StereoAnalyzer sa;

	// stereo spectrum
	viz::StereoSpectrum<BarType> &ss;
	std::optional<sf::BlendMode> spectrum_bm;

	// scope
	viz::ScopeDrawable<sf::RectangleShape> scope;
	std::vector<float> left_channel; // NEEDS to be updated when the scope is updated

	// particle system
	viz::ParticleSystem<ParticleShapeType> &ps;

	// metadata-related fields
	sf::Font font;
	sf::Text title_text{font}, artist_text{font};
	viz::SongMetadataDrawable metadata{title_text, artist_text};

	// timing text
	sf::Text timing_text{font};
	std::ostringstream tt_ss;
	bool tt_enabled{};

#ifdef AUDIOVIZ_PORTAUDIO
	// PortAudio stuff for live playback
	std::optional<pa::PortAudio> pa_init;
	std::optional<pa::Stream> pa_stream;
#endif

	std::vector<viz::Layer> layers;
	tt::RenderTexture final_rt;

	sf::Texture video_bg;

public:
	// need to do this outside of the constructor otherwise the texture is broken?
	void use_attached_pic_as_bg();

	/**
	 * @param size size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media_url url to media source. must contain an audio stream
	 * @param fa reference to your own `tt::FrequencyAnalyzer`
	 * @param ss reference to your own `viz::StereoSpectrum<BarType>`
	 * @param antialiasing antialiasing level to use for round shapes
	 */
	audioviz(
		sf::Vector2u size,
		const std::string &media_url,
		tt::FrequencyAnalyzer &fa,
		viz::StereoSpectrum<BarType> &ss,
		viz::ParticleSystem<ParticleShapeType> &ps,
		int antialiasing = 4);

	/**
	 * Add default effects to the `bg`, `spectrum`, and `particles` layers, if they exist.
	 */
	void add_default_effects();

	/**
	 * Prepare a frame to be drawn with `draw()`.
	 * This method does all the work to produce a frame.
	 * @return Whether another frame can be prepared
	 */
	bool prepare_frame();

	void draw(sf::RenderTarget &target, sf::RenderStates) const override;

	viz::Layer &add_layer(const std::string &name, int antialiasing);
	viz::Layer *get_layer(const std::string &name);

#ifdef AUDIOVIZ_PORTAUDIO
	void set_audio_playback_enabled(bool);
#endif

	void set_timing_text_enabled(bool);

	// important if you are capturing frames for video encoding!
	int get_framerate() const { return framerate; }
	void set_framerate(int framerate);

	// set background image with optional effects: blur and color-multiply
	void set_background(const sf::Texture &texture);

	// set margins around the output size for the spectrum to respect
	void set_spectrum_margin(int margin);

	// set blend mode for spectrum against target
	void set_spectrum_blendmode(const sf::BlendMode &);

	void set_album_cover(const std::string &image_path, sf::Vector2f size = {150, 150});

	// you **must** call this method in order to see text metadata!
	void set_text_font(const std::string &path);

	// void set_media_url(const std::string &url);
	const std::string get_media_url() const;

	/**
	 * Set the number of audio samples used for frequency analysis.
	 * @note Calls `set_fft_size` on the `tt::FrequencyAnalyzer` you passed in the constructor.
	 */
	void set_fft_size(int fft_size);

private:
	void metadata_init();
	void draw_spectrum();
	void draw_particles();
	void play_audio();
	void capture_elapsed_time(const std::string &label, const sf::Clock &_clock);
	void layers_init(int);
	void perform_fft();
};
