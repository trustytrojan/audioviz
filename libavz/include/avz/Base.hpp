#pragma once

#include "Layer.hpp"
#include "Profiler.hpp"
#include "RenderTexture.hpp"

#include <SFML/Graphics.hpp>
#include <memory>
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

namespace avz
{

/**
 * Base class containing the boilerplate for an avz visualizer.
 * Extend this class to start building your own visualizer!
 */
class Base : public sf::Drawable
{
public:
	// avz output size. cannot be changed, so make sure your window is not resizable.
	const sf::Vector2u size;

protected:
	bool profiler_enabled{};
	Profiler profiler;
	sf::Font font;

private:
	std::vector<std::unique_ptr<Layer>> layers;
	RenderTexture final_rt;
	sf::Text profiler_text{font};

public:
	Base(sf::Vector2u size);

	// Emplace a derived Layer type in-place and return a pointer to it.
	template <typename T, typename... Args>
	T &emplace_layer(Args &&...args)
	{
		static_assert(std::is_base_of_v<Layer, T>, "T must derive from Layer");
		auto up = std::make_unique<T>(std::forward<Args>(args)...);
		T *ptr = up.get();
		layers.emplace_back(std::move(up));
		return *ptr;
	}

	// Find a layer by name (non-owning raw pointer). Returns nullptr if not found.
	Layer *get_layer(const std::string &name);

	/**
	 * Get a layer by name with automatic type casting.
	 * @tparam T The derived Layer type to cast to
	 * @param name The name of the layer
	 * @returns A shared_ptr<T> if the layer exists and is of type T, nullptr otherwise
	 *
	 * Usage:
	 * auto my_layer = get_layer_as<MyLayerType>("my_layer");
	 */
	template <typename T>
	T *get_layer_as(const std::string &name)
	{
		return dynamic_cast<T *>(get_layer(name));
	}

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

} // namespace avz
