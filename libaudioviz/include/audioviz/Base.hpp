#pragma once

#include "audioviz/Profiler.hpp"
#include <SFML/Graphics.hpp>

#include <audioviz/Layer.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/media/Media.hpp>
#include <span>
#include <string>
#include <vector>

#define capture_time(label, code)     \
	if (profiler_enabled)             \
	{                                 \
		profiler.startSection(label); \
		code;                         \
		profiler.endSection();        \
	}                                 \
	else                              \
		code;

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

protected:
	// audio frames per video frame
	int afpvf{};
	int audio_frames_needed{};
	bool profiler_enabled{};
	Profiler profiler;
	sf::Font font;

private:
	std::vector<Layer> layers;
	std::vector<const sf::Drawable *> final_drawables;
	std::vector<Layer::DrawCall> final_drawables2;
	int audio_sample_rate{};
	int framerate{60};
	RenderTexture final_rt;
	sf::Text profiler_text{font};

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
	void add_final_drawable2(const sf::Drawable &, sf::RenderStates);

	/**
	 * Prepare the next frame to be drawn with `draw()`. Runs all layers.
	 * @param audio_buffer A span of the audio data for this frame.
	 * @returns Whether another frame can be prepared
	 */
	bool next_frame(std::span<const float> audio_buffer, int num_channels);

	void draw(sf::RenderTarget &, sf::RenderStates) const override;

	// important if you are capturing frames for video encoding!
	void set_framerate(int framerate);
	void set_samplerate(int samplerate);
	inline int get_framerate() const { return framerate; }

	inline void set_font(const std::string &path) { font = sf::Font{path}; }
	inline void enable_profiler() { profiler_enabled = true; }

	// users MUST call this to specify how much audio they need for their visualizers
	// this is an overreach, only used in the terminal methods, make this a parameter of those instead
	inline void set_audio_frames_needed(int needed) { audio_frames_needed = needed; }

	// quick way to start your viz in a window!!!!!!!!
	void start_in_window(Media &media, const std::string &window_title);

	// render this viz to a video file!!!!!!!!
	void encode(
		Media &media,
		const std::string &outfile,
		const std::string &vcodec = "h264",
		const std::string &acodec = "copy");

protected:
	virtual void update(std::span<const float> audio_buffer) {}
};

} // namespace audioviz
