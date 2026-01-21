#include "ScopeDrawable.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace avz
{

ScopeDrawable::ScopeDrawable(const ColorSettings &color, const bool backwards)
	: color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
}

ScopeDrawable::ScopeDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards)
	: rect{rect},
	  color{color},
	  backwards{backwards}
{
	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
	update_shapes();
}

void ScopeDrawable::update(std::span<const float> audio, int sample_rate)
{
	set_sample_rate(sample_rate);
	update(audio);
}

void ScopeDrawable::update(std::span<const float> audio)
{
	if (shape.count <= 0)
		return;

	const int required = required_sample_count();
	std::span<const float> window = audio;

	if (required > 0)
	{
		m_window.assign(required, 0.f);
		if (!audio.empty())
		{
			if ((int)audio.size() >= required)
			{
				const auto start = audio.end() - required;
				std::copy(start, audio.end(), m_window.begin());
			}
			else
			{
				const size_t offset = required - audio.size();
				std::copy(audio.begin(), audio.end(), m_window.begin() + offset);
			}
		}
		window = m_window;
	}

	m_resampled.resize(shape.count);
	if (window.empty())
	{
		std::fill(m_resampled.begin(), m_resampled.end(), 0.f);
	}
	else if (window.size() == 1 || shape.count == 1)
	{
		std::fill(m_resampled.begin(), m_resampled.end(), window[0]);
	}
	else
	{
		for (int i = 0; i < shape.count; ++i)
		{
			const float t = static_cast<float>(i) / (shape.count - 1);
			const float pos = t * (window.size() - 1);
			const size_t idx = static_cast<size_t>(pos);
			const float frac = pos - idx;
			const float a = window[idx];
			const float b = (idx + 1 < window.size()) ? window[idx + 1] : window[idx];
			m_resampled[i] = a * (1.f - frac) + b * frac;
		}
	}

	update_vertices(m_resampled);
}

void ScopeDrawable::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
	target.draw(vertex_array, states);
}

void ScopeDrawable::update_shapes()
{
	shape.count = rect.size.x / (shape.width + shape.spacing);
	if (shape.count <= 0)
	{
		vertex_array.resize(0);
		return;
	}

	vertex_array.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
	vertex_array.resize(shape.count * 6 - 2);
	update_shape_colors();
}

void ScopeDrawable::update_shape_colors()
{
	if (shape.count <= 0)
		return;

	sf::Color prev_color = sf::Color::White;
	for (int i = 0; i < shape.count; ++i)
	{
		const sf::Color shape_color = color.calculate_color((float)i / shape.count);
		if (i == 0)
		{
			vertex_array[0].color = shape_color;
			vertex_array[1].color = shape_color;
			vertex_array[2].color = shape_color;
			vertex_array[3].color = shape_color;
		}
		else
		{
			const int base = 6 * i - 2;
			vertex_array[base].color = prev_color;
			vertex_array[base + 1].color = shape_color;
			vertex_array[base + 2].color = shape_color;
			vertex_array[base + 3].color = shape_color;
			vertex_array[base + 4].color = shape_color;
			vertex_array[base + 5].color = shape_color;
		}

		prev_color = shape_color;
	}
}

int ScopeDrawable::get_shape_vertex_index(int shape_idx, int vertex_num) const
{
	if (shape_idx == 0)
		return vertex_num;

	const int base = 6 * shape_idx - 2;
	return base + vertex_num;
}

int ScopeDrawable::required_sample_count() const
{
	if (audio_duration <= 0.f || sample_rate <= 0)
		return 0;

	const auto required = static_cast<int>(std::round(audio_duration * sample_rate));
	return std::max(1, required);
}

void ScopeDrawable::update_vertices(std::span<const float> samples)
{
	const float half_height = rect.size.y / 2.f;
	const float center = rect.position.y + half_height;
	const bool update_colors = (color.wheel.rate != 0);

	float prev_right = 0.f;
	float prev_bottom = center;
	sf::Color prev_color = sf::Color::White;

	for (int i = 0; i < shape.count; ++i)
	{
		const float audio_val = samples[i];
		const float target = std::clamp(
			center - (half_height * audio_val), (float)rect.position.y, (float)(rect.position.y + rect.size.y));

		float top = target;
		float bottom = target + shape.width;
		if (fill_in)
		{
			top = std::min(center, target);
			bottom = std::max(center, target);
		}

		// clang-format off
		const float x = backwards
			? rect.position.x + rect.size.x - shape.width - i * (shape.width + shape.spacing)
			: rect.position.x + i * (shape.width + shape.spacing);
		// clang-format on
		const float left = x;
		const float right = x + shape.width;

		const sf::Color shape_color = update_colors ? color.calculate_color((float)i / shape.count)
													: vertex_array[get_shape_vertex_index(i, 0)].color;

		if (i == 0)
		{
			vertex_array[0].position = sf::Vector2f(left, top);
			vertex_array[1].position = sf::Vector2f(left, bottom);
			vertex_array[2].position = sf::Vector2f(right, top);
			vertex_array[3].position = sf::Vector2f(right, bottom);

			vertex_array[0].color = shape_color;
			vertex_array[1].color = shape_color;
			vertex_array[2].color = shape_color;
			vertex_array[3].color = shape_color;
		}
		else
		{
			const int base = 6 * i - 2;
			vertex_array[base].position = sf::Vector2f(prev_right, prev_bottom);
			vertex_array[base].color = prev_color;

			vertex_array[base + 1].position = sf::Vector2f(left, top);
			vertex_array[base + 2].position = sf::Vector2f(left, top);
			vertex_array[base + 3].position = sf::Vector2f(left, bottom);
			vertex_array[base + 4].position = sf::Vector2f(right, top);
			vertex_array[base + 5].position = sf::Vector2f(right, bottom);

			vertex_array[base + 1].color = shape_color;
			vertex_array[base + 2].color = shape_color;
			vertex_array[base + 3].color = shape_color;
			vertex_array[base + 4].color = shape_color;
			vertex_array[base + 5].color = shape_color;
		}

		prev_right = right;
		prev_bottom = bottom;
		prev_color = shape_color;
	}
}

} // namespace avz
