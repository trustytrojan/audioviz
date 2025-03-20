#pragma once

#include <SFML/Graphics.hpp>

#ifdef AUDIOVIZ_PORTAUDIO
#include <portaudio.hpp>
#endif

#include <audioviz/Layer.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/media/Media.hpp>

namespace audioviz
{

/**
 * Base class containing the boilerplate for an audioviz visualizer.
 * Extend this class to start building your own visualizer!
 */
class Base : public sf::Drawable
{
public:
	// audioviz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;
	std::vector<Layer> layers;

protected:
	media::Media *const media;
	sf::Font font;

private:
	std::vector<const sf::Drawable *> final_drawables;
	int framerate{60};
	int audio_frames_needed{};
	int afpvf{media->astream().sample_rate() / framerate}; // audio frames per video frame
	RenderTexture final_rt;
	sf::Text timing_text{font};
	std::ostringstream tt_ss;
	bool tt_enabled{};
#ifdef AUDIOVIZ_PORTAUDIO
	// PortAudio stuff for live playback
	pa::PortAudio pa_init;
	pa::Stream pa_stream{0, 2, paFloat32, media->astream().sample_rate(), afpvf};
	bool audio_enabled{true};
#endif

public:
	/**
	 * @param size Size of the output; recommended to match your `sf::RenderTarget`'s size
	 * @param media Pointer to `Media` object. This `base_audioviz` instance will own the object.
	 */
	Base(sf::Vector2u size, media::Media *media);

	// media needs to be freed
	// we will take ownership of it for now
	~Base();

	/// layer api

	Layer &add_layer(const std::string &name, int antialiasing = 0);
	Layer *get_layer(const std::string &name);
	void remove_layer(const std::string &name);

	void add_final_drawable(const sf::Drawable &);

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

	void perform_fft(fft::FrequencyAnalyzer &fa, fft::AudioAnalyzer &aa);

	// users MUST call this to specify how much audio they need for their visualizers
	inline void set_audio_frames_needed(int needed) { audio_frames_needed = needed; }

	// quick way to start your viz in a window!!!!!!!!
	void start_in_window(const std::string &window_title);

	// render this viz to a video file!!!!!!!!
	// not implemented yet: just steal it from ttviz
	void encode(const std::string &outfile, const std::string &vcodec = "h264", const std::string &acodec = "copy");

protected:
	void capture_elapsed_time(const std::string &label, const sf::Clock &);

private:
	void play_audio();
};

} // namespace audioviz
