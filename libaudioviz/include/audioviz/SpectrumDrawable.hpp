#pragma once

#include "audioviz/fft/BinPacker.hpp"
#include "audioviz/fft/Interpolator.hpp"
#include <SFML/Graphics.hpp>
#include <audioviz/ColorSettings.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/util.hpp>
#include <span>

namespace audioviz
{

/**
 * A customizable frequency spectrum visualizer using a single mesh for efficient rendering.
 * Uses sf::TriangleStrip for batched drawing of all bars in a single draw call.
 */
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;
	float multiplier{1};
	std::vector<float, aligned_allocator<float>> m_spectrum;
	sf::VertexArray vertex_array;
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};

	struct
	{
		int width{10}, spacing{5}, count{};
	} bar;

public:
	SpectrumDrawable(const ColorSettings &color, const bool backwards = false);
	SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false);

	inline int get_bar_spacing() const { return bar.spacing; }
	inline int get_bar_count() const { return bar.count; }

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline void set_multiplier(const float multiplier) { this->multiplier = multiplier; }

	void set_rect(const sf::IntRect &rect);
	void set_bar_width(const int width);
	void set_bar_spacing(const int spacing);
	void set_backwards(const bool b);

	void update_bar_colors();
	// bin-packing, interpolating overload of update
	void update(FrequencyAnalyzer &fa, AudioAnalyzer &aa, BinPacker &bp, Interpolator &ip);
	void update(std::span<const float> spectrum);
	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;

private:
	int get_bar_vertex_index(int bar_idx, int vertex_num) const;
	void update_bars();
};

} // namespace audioviz
