#pragma once

#include <SFML/Graphics.hpp>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#include "media/Media.hpp"
#include "viz/Layer.hpp"
#include "tt/AudioAnalyzer.hpp"

class base_audioviz : public sf::Drawable
{
public:
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;
	std::vector<viz::Layer> layers;

protected:
	std::unique_ptr<media::Media> media;
	sf::Font font;

private:
	int framerate{60};
	int audio_frames_needed{};
	int afpvf{media->astream().sample_rate() / framerate}; // audio frames per video frame
	tt::RenderTexture final_rt;
	sf::Text timing_text{font};
	std::ostringstream tt_ss;
	bool tt_enabled{};
#ifdef AUDIOVIZ_PORTAUDIO
	// PortAudio stuff for live playback
	std::optional<pa::PortAudio> pa_init;
	std::optional<pa::Stream> pa_stream;
#endif

public:
	/**
	 * @param size Size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media Pointer to `Media` object. This `base_audioviz` instance will own the object.
	 */
	base_audioviz(sf::Vector2u size, media::Media *media);

	/// layer api

	viz::Layer &add_layer(const std::string &name, int antialiasing = 0);
	viz::Layer *get_layer(const std::string &name);
	void remove_layer(const std::string &name);

	/**
	 * Prepare the next frame to be drawn with `draw()`. Runs all layers.
	 * @param audio_frames Number of audio frames to buffer from media source
	 *                     in preparation for subclass processing.
	 * @returns Whether another frame can be prepared
	 */
	bool next_frame();

	void draw(sf::RenderTarget &, sf::RenderStates) const override;

#ifdef AUDIOVIZ_PORTAUDIO
	void set_audio_playback_enabled(bool);
#endif

	inline void set_timing_text_enabled(const bool enabled) { tt_enabled = enabled; }

	// important if you are capturing frames for video encoding!
	void set_framerate(int framerate);
	inline int get_framerate() const { return framerate; }

	// must be called for timing text to display
	inline void set_text_font(const std::string &path) { font = sf::Font{path}; }

	inline std::string get_media_url() const { return media->url; }

	void perform_fft(tt::FrequencyAnalyzer &fa, tt::AudioAnalyzer &aa);

	// users MUST call this to specify how much audio they need for their visualizers
	void set_audio_frames_needed(int needed) { audio_frames_needed = needed; }

protected:
	void capture_elapsed_time(const std::string &label, const sf::Clock &);

private:
	void play_audio();
};
