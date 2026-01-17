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
	bool profiler_enabled{};
	Profiler profiler;
	sf::Font font;

private:
	std::vector<Layer> layers;
	std::vector<const sf::Drawable *> final_drawables;
	std::vector<Layer::DrawCall> final_drawables2;
	RenderTexture final_rt;
	sf::Text profiler_text{font};

public:
	Base(sf::Vector2u size);

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
	void next_frame(std::span<const float> audio_buffer);

	void draw(sf::RenderTarget &, sf::RenderStates) const override;

	inline void set_font(const std::string &path) { font = sf::Font{path}; }
	inline void enable_profiler() { profiler_enabled = true; }

protected:
	virtual void update(std::span<const float> audio_buffer) {}
};

} // namespace audioviz
