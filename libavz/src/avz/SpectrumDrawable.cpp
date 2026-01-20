#include <algorithm>
#include <avz/SpectrumDrawable.hpp>
#include <cassert>

namespace audioviz
{

SpectrumDrawable::SpectrumDrawable(const ColorSettings &color, const bool backwards)
	: color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
}

SpectrumDrawable::SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards)
	: rect{rect},
	  color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
	update_bars();
}

void SpectrumDrawable::set_rect(const sf::IntRect &rect)
{
	if (this->rect == rect)
		return;
	this->rect = rect;
	update_bars();
}

void SpectrumDrawable::set_bar_width(const int width)
{
	if (bar.width == width)
		return;
	bar.width = width;
	update_bars();
}

void SpectrumDrawable::set_bar_spacing(const int spacing)
{
	if (bar.spacing == spacing)
		return;
	bar.spacing = spacing;
	update_bars();
}

void SpectrumDrawable::set_backwards(const bool b)
{
	if (backwards == b)
		return;
	backwards = b;
	update_bars();
}

void SpectrumDrawable::update(FrequencyAnalyzer &fa, AudioAnalyzer &aa, BinPacker &bp, Interpolator &ip)
{
	m_spectrum.resize(bar.count);
	bp.bin_pack(m_spectrum, aa.compute_amplitudes(fa));
	if (bp.get_scale() != BinPacker::Scale::LINEAR)
		ip.interpolate(m_spectrum);
	update(m_spectrum);
}

void SpectrumDrawable::update(std::span<const float> spectrum)
{
	assert(spectrum.size() >= bar.count);

	// Update vertex heights and colors
	const bool update_colors = (color.wheel.rate != 0);

	for (int i = 0; i < bar.count; ++i)
	{
		const float height = std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y);
		const int tl = get_bar_vertex_index(i, 0);
		const int bl = get_bar_vertex_index(i, 1);
		const int tr = get_bar_vertex_index(i, 2);
		const int br = get_bar_vertex_index(i, 3);
		const sf::Color bar_color =
			update_colors ? color.calculate_color((float)i / bar.count) : vertex_array[tl].color;

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

void SpectrumDrawable::draw(sf::RenderTarget &target, sf::RenderStates states) const
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

void SpectrumDrawable::update_bar_colors()
{
	for (int i = 0; i < bar.count; ++i)
	{
		const sf::Color bar_color = color.calculate_color((float)i / bar.count);

		for (int v = 0; v < 4; ++v)
		{
			const int idx = get_bar_vertex_index(i, v);
			vertex_array[idx].color = bar_color;
		}
		if (i > 0)
			vertex_array[6 * i].color = bar_color;
	}
}

int SpectrumDrawable::get_bar_vertex_index(int bar_idx, int vertex_num) const
{
	// Vertex layout:
	// Bar 0: [0..3] => TL, BL, TR, BR
	// Bar i>0 adds 6 vertices: BR_prev(dup), TL, TL(dup), BL, TR, BR
	if (bar_idx == 0)
		return vertex_num;
	switch (vertex_num)
	{
	case 0:
		return 6 * bar_idx - 1; // TL
	case 1:
		return 6 * bar_idx + 1; // BL
	case 2:
		return 6 * bar_idx + 2; // TR
	case 3:
		return 6 * bar_idx + 3; // BR
	default:
		return 0;
	}
}

void SpectrumDrawable::update_bars()
{
	bar.count = rect.size.x / (bar.width + bar.spacing);
	if (bar.count <= 0)
	{
		vertex_array.resize(0);
		return;
	}

	// Vertex count: 4 + (bar.count - 1) * 6 == bar.count * 6 - 2
	vertex_array.resize(bar.count * 6 - 2);

	const float bottom = rect.position.y + rect.size.y;
	const float top = bottom; // will be updated per-frame

	int out = 0;
	float prev_right = 0.f;
	sf::Color prev_color = sf::Color::White;

	for (int i = 0; i < bar.count; ++i)
	{
		const float x = backwards ? rect.position.x + rect.size.x - bar.width - i * (bar.width + bar.spacing)
								  : rect.position.x + i * (bar.width + bar.spacing);
		const float left = x;
		const float right = x + bar.width;
		const sf::Color bar_color = color.calculate_color((float)i / bar.count);

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
