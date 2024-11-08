#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>


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
	bool vert = false;

	sf::Angle angle = sf::degrees(0);
	sf::Transformable tf;

public:
	std::vector<ShapeType> shapes;

	ScopeDrawable(const sf::IntRect &rect, const bool backwards = false)
		: rect{rect},
		  backwards{backwards}
	{
		update_shape_x_positions();

		const sf::Vector2f _origin{rect.size.x / 2.f, rect.size.y / 2.f};
		tf.setOrigin(_origin);
		tf.setPosition(_origin);
		tf.setRotation(angle);
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

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shape_x_positions();
	}

	void set_backwards(bool b) { backwards = b; }

	void set_rotation_angle(sf::Angle angle) { tf.setRotation(angle); }

	void set_center_point(double radius, sf::Angle angle)
	{
		sf::Vector2f origin_{rect.size.x/2.f, rect.size.y/2.f};	
		sf::Vector2f coord{origin_.x + radius * cos(angle.asRadians()), origin_.y - radius * sin(angle.asRadians())};

		tf.setPosition({coord});
	}

	size_t get_shape_count() const { return shapes.size(); }

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
