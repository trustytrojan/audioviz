#pragma once

#include <SFML/Graphics.hpp>

#include <audioviz/Layer.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/media/Media.hpp>
#include <limits>
#include <map>
#include <span>
#include <string>

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
	sf::Font font;

private:
	std::vector<const sf::Drawable *> final_drawables;
	int audio_sample_rate{};
	int framerate{60};

protected:
	// audio frames per video frame
	int afpvf{};
	int audio_frames_needed{};

private:
	RenderTexture final_rt;

	sf::Text timing_text{font};
	struct TimingStat
	{
		float min{std::numeric_limits<float>::max()};
		float max{0.0f};
		float total{0.0f};
		size_t count{0};
		float current{0.0f};
		float avg() const { return (count == 0) ? 0.0f : total / count; }
	};
	std::map<std::string, TimingStat> timing_stats;
	bool tt_enabled{};

public:
	/**
	 * @param size Size of the output; recommended to match your `sf::RenderTarget`'s size
	 */
	Base(sf::Vector2u size);

	/// layer api

	Layer &add_layer(const std::string &name, int antialiasing = 0);
	Layer *get_layer(const std::string &name);
	void remove_layer(const std::string &name);

	void add_final_drawable(const sf::Drawable &);

	/**
	 * Prepare the next frame to be drawn with `draw()`. Runs all layers.
	 * @param audio_buffer A span of the audio data for this frame.
	 * @returns Whether another frame can be prepared
	 */
	bool next_frame(std::span<const float> audio_buffer);

	void draw(sf::RenderTarget &, sf::RenderStates) const override;

	inline void set_timing_text_enabled(const bool enabled) { tt_enabled = enabled; }
	inline bool timing_text_enabled() { return tt_enabled; }

	// important if you are capturing frames for video encoding!
	void set_framerate(int framerate);
	void set_samplerate(int samplerate);
	inline int get_framerate() const { return framerate; }

	// must be called for timing text to display
	inline void set_text_font(const std::string &path) { font = sf::Font{path}; }

	// users MUST call this to specify how much audio they need for their visualizers
	inline void set_audio_frames_needed(int needed) { audio_frames_needed = needed; }

	// quick way to start your viz in a window!!!!!!!!
	void start_in_window(Media &media, const std::string &window_title);

	// render this viz to a video file!!!!!!!!
	// not implemented yet: just steal it from ttviz
	void encode(
		Media &media,
		const std::string &outfile,
		const std::string &vcodec = "h264",
		const std::string &acodec = "copy");

protected:
	void capture_elapsed_time(const std::string &label, const sf::Clock &);
	virtual void update(std::span<const float> /*audio_buffer*/) {}
};

} // namespace audioviz
