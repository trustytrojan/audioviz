#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>

#include "ColorSettings.hpp"

namespace viz
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
	sf::Angle angle = sf::degrees(0);
	sf::Transformable tf;
	std::vector<ShapeType> shapes;

public:
	ScopeDrawable()
	{
		for (auto &shape : shapes)
		{
			shape.setRadius(10);
			shape.setFillColor(sf::Color::White);
		}
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

	void set_fill_in(const bool b) { fill_in = b; }
	void set_backwards(const bool b) { backwards = b; }
	void set_rotation_angle(const sf::Angle angle) { tf.setRotation(angle); }

	void set_center_point(const double radius, const sf::Angle angle)
	{
		sf::Vector2f origin_{rect.size.x / 2.f, rect.size.y / 2.f};
		sf::Vector2f coord{origin_.x + radius * cos(angle.asRadians()), origin_.y - radius * sin(angle.asRadians())};
		tf.setPosition({coord});
	}

	size_t get_shape_count() const { return shapes.size(); }

	void update(const std::span<float> &audio)
	{
		assert(audio.size() >= shapes.size());

		for (int i = 0; i < (int)shapes.size(); ++i)
		{
			const auto half_height = rect.size.y / 2.f;
			const auto half_heightx = rect.size.x / 2.f;

			shapes[i].setFillColor(color.calculate_color((float)i / shapes.size()));

			if (!fill_in)
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height + (-half_height * audio[i]), 0.f, (float)rect.size.y),
				});
			}
			else
			{
				shapes[i].setPosition({
					shapes[i].getPosition().x,
					std::clamp(half_height, 0.f, (float)rect.size.y),
				});
				shapes[i].setSize({shape.width, (-half_height * audio[i])});
			}
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &shape : shapes)
			target.draw(shape, tf.getTransform());
	}

private:
	void update_shapes()
	{
		const int shape_count = rect.size.x / (shape.width + shape.spacing);
		shapes.resize(shape_count);

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
			// const auto y = backwards
			// 	? rect.position.y + rect.size.y - shape.width - i * (shape.width + shape.spacing)
			// 	: rect.position.y + i * (shape.width + shape.spacing);
			// clang-format on
			// if (vert)
			// shapes[i].setPosition({rect.position.x + rect.size.x / 2, y});
			// else
			shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});

			// shapes[i].setPosition({rect.position.x + rect.size.x / 2, y});
		}
	}
};

} // namespace viz
