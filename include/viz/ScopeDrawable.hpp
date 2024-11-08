#pragma once

#include <SFML/Graphics.hpp>
#include "ColorSettings.hpp"

namespace viz
{

template <typename ShapeType>
class ScopeDrawable : public sf::Drawable
{
	std::vector<ShapeType> shapes;
	sf::IntRect rect;
	bool backwards = false;
	struct
	{
		int width = 10, spacing = 5;
	} shape;
	bool fill_in = false;
	bool vert = false;

	ColorSettings &color;

public:

	enum class ColorMode
	{
		WHEEL_RANGES,
		WHEEL,
		WHEEL_RANGES_REVERSE,
		SOLID
	};

	ScopeDrawable(const sf::IntRect &rect, ColorSettings &color, const bool backwards = false)
		: rect{rect},
		  color{color},
		  backwards{backwards}
	{
		update_shape_x_positions();
	}

	void set_shape_spacing(const int spacing)
	{
		if (shape.spacing == spacing)
			return;
		shape.spacing = spacing;
		update_shape_x_positions();
	}

	void set_shape_width(const int width)
	{
		if (shape.width == width)
			return;
		shape.width = width;
		update_shape_x_positions();
	}

	void set_fill_in(bool _fill) { fill_in = _fill; }

	void set_vert(bool _vert) { vert = _vert; }
	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shape_x_positions();
	}

	void set_backwards(bool b) { backwards = b; }

	size_t get_shape_count() const { return shapes.size(); }

	void color_wheel_increment()
	{
		color.wheel.increment_time();
		for (int i = 0; i < (int)shapes.size(); ++i) 
			shapes[i].setFillColor(color.calculate_color((float)i / shapes.size())); 
	}

	void update_shape_positions(const std::vector<float> &audio)
	{
		if (shapes.size() != audio.size())
		{
			shapes.resize(audio.size());
			update_shape_x_positions();
		}

		for (int i = 0; i < (int)audio.size(); ++i)
		{
			const auto half_height = rect.size.y / 2.f;
			const auto half_heightx = rect.size.x / 2.f;

			if (!fill_in && !vert)
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height + (-half_height * audio[i]), 0.f, (float)rect.size.y),
				});
			}
			else if (fill_in && !vert)
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height, 0.f, (float)rect.size.y),
				});
				shapes[i].setSize({shape.width, (-half_height * audio[i])});
			}
			else if (!fill_in && vert)
			{
				shapes[i].setPosition({
					std::clamp(half_heightx + (-half_heightx * audio[i]), 0.f, (float)rect.size.x),
					shapes[i].getPosition().y,
				});
			}
			else
			{
				shapes[i].setPosition({
					std::clamp(half_heightx, 0.f, (float)rect.size.x),
					shapes[i].getPosition().y,
				});
				shapes[i].setSize({(-half_height * audio[i]), shape.width});
			}
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (int i = 0; i < (int)shapes.size(); ++i) {
			target.draw(shapes[i], states);
		}
	}

private:
	void update_shape_x_positions()
	{
		for (int i = 0; i < shapes.size(); ++i)
		{
			if constexpr (std::is_base_of_v<sf::CircleShape, ShapeType>)
				shapes[i].setRadius(shape.width);
			else if constexpr (std::is_base_of_v<sf::RectangleShape, ShapeType>)
				shapes[i].setSize({shape.width, shape.width});
			else
				shapes[i].setScale(shape.width);

			shapes[i].setFillColor(color.calculate_color((float)i / shapes.size()));

			// clang-format off
			const auto x = backwards
				? rect.position.x + rect.size.x - shape.width - i * (shape.width + shape.spacing)
				: rect.position.x + i * (shape.width + shape.spacing);
			const auto y = backwards
				? rect.position.y + rect.size.y - shape.width - i * (shape.width + shape.spacing)
				: rect.position.y + i * (shape.width + shape.spacing);
			// clang-format on
			if (vert)
				shapes[i].setPosition({rect.position.x + rect.size.x / 2, y});
			else
				shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});
		}
	}
};

} // namespace viz
