#include <audioviz/SpectrumDrawable_new.hpp>
#include <cassert>
#include <algorithm>

#ifdef AUDIOVIZ_IMGUI
#include "imgui.h"
#endif

namespace audioviz
{

SpectrumDrawable_new::SpectrumDrawable_new(const ColorSettings &color, const bool backwards)
	: color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
}

SpectrumDrawable_new::SpectrumDrawable_new(const sf::IntRect &rect, const ColorSettings &color, const bool backwards)
	: rect{rect},
	  color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
	update_bars();
}

void SpectrumDrawable_new::set_rect(const sf::IntRect &rect)
{
	if (this->rect == rect)
		return;
	this->rect = rect;
	update_bars();
}

void SpectrumDrawable_new::set_bar_width(const int width)
{
	if (bar.width == width)
		return;
	bar.width = width;
	update_bars();
}

void SpectrumDrawable_new::set_bar_spacing(const int spacing)
{
	if (bar.spacing == spacing)
		return;
	bar.spacing = spacing;
	update_bars();
}

void SpectrumDrawable_new::set_backwards(const bool b)
{
	if (backwards == b)
		return;
	backwards = b;
	update_bars();
}

void SpectrumDrawable_new::set_bar_count(int desired_count)
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

void SpectrumDrawable_new::update(FrequencyAnalyzer &fa, AudioAnalyzer &aa, int channel)
{
	m_spectrum.resize(bar_count);
	fa.bin_pack(m_spectrum, aa.get_channel_data(channel).fft_amplitudes);
	fa.interpolate(m_spectrum);
	update(m_spectrum);
}

void SpectrumDrawable_new::update(std::span<const float> spectrum)
{
	assert(spectrum.size() >= bar_count);
	
	// Update vertex heights and colors
	const bool update_colors = (color.wheel.rate != 0);
	
	for (int i = 0; i < bar_count; ++i)
	{
		const float height = std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y);
		const int tl = get_bar_vertex_index(i, 0);
		const int bl = get_bar_vertex_index(i, 1);
		const int tr = get_bar_vertex_index(i, 2);
		const int br = get_bar_vertex_index(i, 3);
		const sf::Color bar_color = update_colors ? color.calculate_color((float)i / bar_count) : vertex_array[tl].color;
		
		const float bottom_y = rect.position.y + rect.size.y;
		const float top_y = bottom_y - height;

		// Per-bar color
		vertex_array[tl].color = bar_color;
		vertex_array[bl].color = bar_color;
		vertex_array[tr].color = bar_color;
		vertex_array[br].color = bar_color;

		// Only top edge moves
		vertex_array[tl].position.y = top_y;
		vertex_array[tr].position.y = top_y;

		// Bars after the first have a duplicated TL vertex to restart the strip.
		// It must match TL exactly, otherwise you'll see only one triangle ("spikes").
		if (i > 0)
		{
			const int tl_dup = 6 * i;
			vertex_array[tl_dup].position.y = top_y;
			vertex_array[tl_dup].color = bar_color;
		}
	}
}

void SpectrumDrawable_new::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(vertex_array, states);
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

#ifdef AUDIOVIZ_IMGUI
void SpectrumDrawable_new::draw_imgui()
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

void SpectrumDrawable_new::update_bar_colors()
{
	for (int i = 0; i < bar_count; ++i)
	{
		const sf::Color bar_color = color.calculate_color((float)i / bar_count);
		
		for (int v = 0; v < 4; ++v)
		{
			const int idx = get_bar_vertex_index(i, v);
			vertex_array[idx].color = bar_color;
		}
		if (i > 0)
			vertex_array[6 * i].color = bar_color;
	}
}

int SpectrumDrawable_new::get_bar_vertex_index(int bar_idx, int vertex_num) const
{
	// Vertex layout:
	// Bar 0: [0..3] => TL, BL, TR, BR
	// Bar i>0 adds 6 vertices: BR_prev(dup), TL, TL(dup), BL, TR, BR
	if (bar_idx == 0)
		return vertex_num;
	switch (vertex_num)
	{
		case 0: return 6 * bar_idx - 1; // TL
		case 1: return 6 * bar_idx + 1; // BL
		case 2: return 6 * bar_idx + 2; // TR
		case 3: return 6 * bar_idx + 3; // BR
		default: return 0;
	}
}

void SpectrumDrawable_new::update_bars()
{
	bar_count = rect.size.x / (bar.width + bar.spacing);
	if (bar_count <= 0)
	{
		vertex_array.resize(0);
		return;
	}
	
	// Vertex count: 4 + (bar_count - 1) * 6 == bar_count * 6 - 2
	vertex_array.resize(bar_count * 6 - 2);

	const float bottom = rect.position.y + rect.size.y;
	const float top = bottom; // will be updated per-frame

	int out = 0;
	float prev_right = 0.f;
	sf::Color prev_color = sf::Color::White;

	for (int i = 0; i < bar_count; ++i)
	{
		const float x = backwards
			? rect.position.x + rect.size.x - bar.width - i * (bar.width + bar.spacing)
			: rect.position.x + i * (bar.width + bar.spacing);
		const float left = x;
		const float right = x + bar.width;
		const sf::Color bar_color = color.calculate_color((float)i / bar_count);

		if (i == 0)
		{
			// First rectangle: TL, BL, TR, BR
			vertex_array[out++] = sf::Vertex(sf::Vector2f(left, top), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(left, bottom), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(right, top), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(right, bottom), bar_color);
		}
		else
		{
			// Degenerate restart: BR_prev(dup), TL, TL(dup), BL, TR, BR
			vertex_array[out++] = sf::Vertex(sf::Vector2f(prev_right, bottom), prev_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(left, top), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(left, top), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(left, bottom), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(right, top), bar_color);
			vertex_array[out++] = sf::Vertex(sf::Vector2f(right, bottom), bar_color);
		}

		prev_right = right;
		prev_color = bar_color;
	}
}

} // namespace audioviz
