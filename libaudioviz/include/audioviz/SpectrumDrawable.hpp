#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/ColorSettings.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/util.hpp>
#include <print>
#include <vector>

#ifdef AUDIOVIZ_IMGUI
#include "imgui.h"
#endif

namespace audioviz
{

/**
 * A customizable frequency spectrum visualizer.
 * @tparam BarType A subclass of `sf::Shape` that implements `setWidth(float)` and `setHeight(float)`.
 */
template <typename BarType>
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;

	// spectrum parameters
	float multiplier{4};

	// internal data
	std::vector<float> m_spectrum;
	std::vector<BarType> bars;
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};

	struct
	{
		int width{10}, spacing{5};
	} bar;

public:
	SpectrumDrawable(const ColorSettings &color, const bool backwards = false)
		: color{color},
		  backwards{backwards}
	{
	}

	SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false)
		: rect{rect},
		  color{color},
		  backwards{backwards}
	{
		update_bars();
	}

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline int get_bar_spacing() const { return bar.spacing; }
	inline void set_multiplier(const float multiplier) { this->multiplier = multiplier; }

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_bars();
	}

	void set_bar_width(const int width)
	{
		if (bar.width == width)
			return;
		bar.width = width;
		update_bars();
	}

	void set_bar_spacing(const int spacing)
	{
		if (bar.spacing == spacing)
			return;
		bar.spacing = spacing;
		update_bars();
	}

	void set_backwards(const bool b)
	{
		if (backwards == b)
			return;
		backwards = b;
		update_bars();
	}

	/**
	 * Set the number of bars to display by calculating appropriate bar width and spacing.
	 * Keeps spacing fixed and calculates the bar width needed to fit the desired count.
	 * @param desired_count Number of bars to display
	 */
	void set_bar_count(int desired_count)
	{
		if (desired_count <= 0 || rect.size.x <= 0)
			return;

		// Calculate available width for bars: total_width - (count - 1) * spacing
		const int total_spacing = (desired_count - 1) * bar.spacing;
		const int available_width = rect.size.x - total_spacing;

		if (available_width <= 0)
			return;

		const int new_width = available_width / desired_count;
		if (new_width > 0)
			set_bar_width(new_width);
	}

	void update(FrequencyAnalyzer &fa, AudioAnalyzer &aa, int channel)
	{
		m_spectrum.resize(bars.size());
		fa.bin_pack(m_spectrum, aa.get_channel_data(channel).fft_amplitudes);
		fa.interpolate(m_spectrum);
		update(m_spectrum);
	}

	void update(std::span<const float> spectrum)
	{
		assert(spectrum.size() >= bars.size());
		if (color.wheel.rate != 0)
			for (int i = 0; i < (int)bars.size(); ++i)
			{
				// setFillColor is expensive, only call it if we need to!
				update_bar_color(i);
				bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
			}
		else
			for (int i = 0; i < (int)bars.size(); ++i)
				bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &bar : bars)
			target.draw(bar, states);
		if (debug_rect)
		{
			// shows the rect of the object
			sf::RectangleShape r{sf::Vector2f{rect.size}};
			r.setFillColor(sf::Color::Transparent);
			r.setOutlineThickness(1);
			r.setPosition(sf::Vector2f{rect.position});

			// shows the origin of the object
			sf::CircleShape c{5};
			c.setPosition(sf::Vector2f{rect.position});

			// draw with states,
			target.draw(r, states);
			target.draw(c, states);

			// then draw without any states in red, to show how any render states may have transformed the object
			r.setOutlineColor(sf::Color::Red);
			c.setFillColor(sf::Color::Red);
			target.draw(r);
			target.draw(c);
		}
	}

	inline int get_bar_count() const { return bars.size(); }

#ifdef AUDIOVIZ_IMGUI
	void draw_imgui()
	{
		// Bar parameters
		int temp_width = bar.width;
		if (ImGui::SliderInt("Bar Width", &temp_width, 1, 50))
			set_bar_width(temp_width);

		int temp_spacing = bar.spacing;
		if (ImGui::SliderInt("Bar Spacing", &temp_spacing, 0, 50))
			set_bar_spacing(temp_spacing);

		// Multiplier
		ImGui::SliderFloat("Multiplier", &multiplier, 0.1f, 20.0f);

		// Backwards toggle
		bool temp_backwards = backwards;
		if (ImGui::Checkbox("Backwards", &temp_backwards))
			set_backwards(temp_backwards);

		// Debug rect toggle
		ImGui::Checkbox("Debug Rect", &debug_rect);

		// Rect position and size
		ImGui::Text("Bounding Box:");
		ImGui::Indent();

		int rect_pos[2] = {rect.position.x, rect.position.y};
		if (ImGui::InputInt2("Position", rect_pos))
		{
			sf::IntRect new_rect = rect;
			new_rect.position = {rect_pos[0], rect_pos[1]};
			set_rect(new_rect);
		}

		int rect_size[2] = {rect.size.x, rect.size.y};
		if (ImGui::InputInt2("Size", rect_size))
		{
			if (rect_size[0] > 0 && rect_size[1] > 0)
			{
				sf::IntRect new_rect = rect;
				new_rect.size = {rect_size[0], rect_size[1]};
				set_rect(new_rect);
			}
		}

		ImGui::Unindent();

		// Display current bar count (read-only)
		ImGui::Text("Bar Count: %d", get_bar_count());

		const auto drag = util::imgui_drag_resize(rect);
		if (drag.moved || drag.resized)
			set_rect(drag.rect);
	}
#endif

	void update_bar_colors()
	{
		for (int i = 0; i < bars.size(); ++i)
			update_bar_color(i);
	}

private:
	// call after changing any property of the spectrum/bars that will change their positions or colors
	void update_bars()
	{
		const int bar_count = rect.size.x / (bar.width + bar.spacing);
		bars.resize(bar_count);

		for (int i = 0; i < bar_count; ++i)
		{
			update_bar_color(i);
			bars[i].setWidth(bar.width);

			// clang-format off
			const auto x = backwards
				? rect.position.x + rect.size.x - bar.width - i * (bar.width + bar.spacing)
				: rect.position.x + i * (bar.width + bar.spacing);
			// clang-format on

			bars[i].setPosition({x, rect.position.y + rect.size.y});
		}
	}

	void update_bar_color(const int i) { bars[i].setFillColor(color.calculate_color((float)i / bars.size())); }
};

} // namespace audioviz
