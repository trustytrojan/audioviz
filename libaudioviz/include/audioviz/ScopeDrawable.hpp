#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <audioviz/ColorSettings.hpp>
#include <audioviz/util.hpp>
#include <span>

#include "imgui.h"

namespace audioviz
{

template <typename ShapeType>
class ScopeDrawable : public sf::Drawable
{
	sf::IntRect rect;
	bool backwards = false;
	struct
	{
		int width = 10, spacing = 5;
	} shape;
	bool fill_in = false;
	const ColorSettings &color;
	std::vector<ShapeType> shapes;

public:
	ScopeDrawable(const ColorSettings &color, const bool backwards = false)
		: color{color},
		  backwards{backwards}
	{
		update_shapes();
	}

	ScopeDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false)
		: rect{rect},
		  color{color},
		  backwards{backwards}
	{
		update_shapes();
	}

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shapes();
	}

	void set_shape_spacing(int spacing)
	{
		if (spacing == shape.spacing)
			return;
		shape.spacing = spacing;
		update_shapes();
	}

	void set_shape_width(int width)
	{
		if (width == shape.width)
			return;
		shape.width = width;
		update_shapes();
	}

	void set_fill_in(const bool b)
	{
		if (fill_in == b)
			return;
		fill_in = b;
		// Reset shape sizes when toggling fill_in mode
		if constexpr (std::is_base_of_v<sf::RectangleShape, ShapeType>)
			for (auto &s : shapes)
				s.setSize({this->shape.width, this->shape.width});
	}

	void set_backwards(const bool b) { backwards = b; }
	size_t get_shape_count() const { return shapes.size(); }

	void update(const std::span<float> &audio)
	{
		for (int i = 0; i < (int)shapes.size(); ++i)
		{
			float audio_val;
			if (audio.empty())
			{
				audio_val = 0.0f;
			}
			else if (shapes.size() == 1)
			{
				audio_val = audio[0];
			}
			else if (audio.size() == shapes.size())
			{
				audio_val = audio[i];
			}
			else
			{
				float t = static_cast<float>(i) / (shapes.size() - 1);
				float pos = t * (audio.size() - 1);
				size_t idx = static_cast<size_t>(pos);
				float frac = pos - idx;
				if (idx + 1 < audio.size())
					audio_val = audio[idx] * (1 - frac) + audio[idx + 1] * frac;
				else
					audio_val = audio[idx];
			}

			const auto half_height = rect.size.y / 2.f;

			shapes[i].setFillColor(color.calculate_color((float)i / shapes.size()));

			// only rectangles have `setSize`, not circles
			if constexpr (std::is_base_of_v<sf::RectangleShape, ShapeType>)
				if (!fill_in)
				{
					shapes[i].setPosition({
						shapes[i].getPosition().x,
						std::clamp(
							rect.position.y + half_height - (half_height * audio_val),
							(float)rect.position.y,
							(float)(rect.position.y + rect.size.y)),
					});
				}
				else
				{
					shapes[i].setPosition({
						shapes[i].getPosition().x,
						std::clamp(
							rect.position.y + half_height,
							(float)rect.position.y,
							(float)(rect.position.y + rect.size.y)),
					});
					shapes[i].setSize({shape.width, (-half_height * audio_val)});
				}
			else
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(
						rect.position.y + half_height - (half_height * audio_val),
						(float)rect.position.y,
						(float)(rect.position.y + rect.size.y)),
				});
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &shape : shapes)
			target.draw(shape, states);
	}

	void draw_imgui()
	{
		// Shape parameters
		int temp_width = shape.width;
		if (ImGui::SliderInt("Shape Width", &temp_width, 1, 50))
			set_shape_width(temp_width);

		int temp_spacing = shape.spacing;
		if (ImGui::SliderInt("Shape Spacing", &temp_spacing, 0, 50))
			set_shape_spacing(temp_spacing);

		// Fill in toggle
		bool temp_fill_in = fill_in;
		if (ImGui::Checkbox("Fill In", &temp_fill_in))
			set_fill_in(temp_fill_in);

		// Backwards toggle
		bool temp_backwards = backwards;
		if (ImGui::Checkbox("Backwards##scope", &temp_backwards))
			set_backwards(temp_backwards);

		// Rect position and size
		ImGui::Text("Bounding Box:");
		ImGui::Indent();

		int rect_pos[2] = {rect.position.x, rect.position.y};
		if (ImGui::InputInt2("Position##scope", rect_pos))
		{
			sf::IntRect new_rect = rect;
			new_rect.position = {rect_pos[0], rect_pos[1]};
			set_rect(new_rect);
		}

		int rect_size[2] = {rect.size.x, rect.size.y};
		if (ImGui::InputInt2("Size##scope", rect_size))
		{
			if (rect_size[0] > 0 && rect_size[1] > 0)
			{
				sf::IntRect new_rect = rect;
				new_rect.size = {rect_size[0], rect_size[1]};
				set_rect(new_rect);
			}
		}

		ImGui::Unindent();

		// Display current shape count (read-only)
		ImGui::Text("Shape Count: %zu", get_shape_count());

		const auto drag = util::imgui_drag_resize(rect);
		if (drag.moved || drag.resized)
			set_rect(drag.rect);
	}

private:
	void update_shapes()
	{
		const int shape_count = rect.size.x / (shape.width + shape.spacing);
		shapes.resize(shape_count);

		for (int i = 0; i < shapes.size(); ++i)
		{
			if constexpr (std::is_base_of_v<sf::CircleShape, ShapeType>)
				shapes[i].setRadius(shape.width / 2.f);
			else if constexpr (std::is_base_of_v<sf::RectangleShape, ShapeType>)
				shapes[i].setSize({shape.width, shape.width});
			else
				shapes[i].setScale(shape.width);

			shapes[i].setFillColor(color.calculate_color((float)i / shapes.size()));

			// clang-format off
			const auto x = backwards
				? rect.position.x + rect.size.x - shape.width - i * (shape.width + shape.spacing)
				: rect.position.x + i * (shape.width + shape.spacing);
			// clang-format on

			shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});
		}
	}
};

} // namespace audioviz
