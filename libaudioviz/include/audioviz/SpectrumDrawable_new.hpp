#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/ColorSettings.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/util.hpp>
#include <vector>
#include <span>

namespace audioviz
{

/**
 * A customizable frequency spectrum visualizer using a single mesh for efficient rendering.
 * Uses sf::TriangleStrip for batched drawing of all bars in a single draw call.
 */
class SpectrumDrawable_new : public sf::Drawable
{
	const ColorSettings &color;

	// spectrum parameters
	float multiplier{4};

	// internal data
	std::vector<float> m_spectrum;
	mutable sf::VertexArray vertex_array;
	
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};
	int bar_count{};

	struct
	{
		int width{10}, spacing{5};
	} bar;

public:
	SpectrumDrawable_new(const ColorSettings &color, const bool backwards = false);
	SpectrumDrawable_new(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false);

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline int get_bar_spacing() const { return bar.spacing; }
	inline void set_multiplier(const float multiplier) { this->multiplier = multiplier; }

	void set_rect(const sf::IntRect &rect);
	void set_bar_width(const int width);
	void set_bar_spacing(const int spacing);
	void set_backwards(const bool b);

	/**
	 * Set the number of bars to display by calculating appropriate bar width and spacing.
	 * Keeps spacing fixed and calculates the bar width needed to fit the desired count.
	 * @param desired_count Number of bars to display
	 */
	void set_bar_count(int desired_count);

	void update(FrequencyAnalyzer &fa, AudioAnalyzer &aa, int channel);
	void update(std::span<const float> spectrum);

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

	inline int get_bar_count() const { return bar_count; }

#ifdef AUDIOVIZ_IMGUI
	void draw_imgui();
#endif

	void update_bar_colors();

private:
	int get_bar_vertex_index(int bar_idx, int vertex_num) const;
	void update_bars();
};

} // namespace audioviz
