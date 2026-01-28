#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/ColorSettings.hpp>
#include <avz/gfx/util.hpp>
#include <span>

namespace avz
{

/**
 * A customizable frequency spectrum visualizer using a single mesh for efficient rendering.
 * Uses sf::TriangleStrip for batched drawing of all bars in a single draw call.
 */
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;
	float multiplier{1};
	sf::VertexArray vertex_array;
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};
	bool use_gs{false};

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
	inline void set_use_gs(bool b)
	{
		if (use_gs == b)
			return;
		use_gs = b;
		update_bars();
	}
	inline bool get_use_gs() const { return use_gs; }

	void set_rect(const sf::IntRect &rect);
	void set_bar_width(const int width);
	void set_bar_spacing(const int spacing);
	void set_backwards(const bool b);

	inline int get_bar_width() const { return bar.width; }
	inline sf::IntRect get_rect() const { return rect; }

	void update_bar_colors();
	void update(std::span<const float> spectrum);
	void draw(sf::RenderTarget &target, sf::RenderStates states = {}) const override;

private:
	int get_bar_vertex_index(int bar_idx, int vertex_num) const;
	void update_bars();
};

} // namespace avz
