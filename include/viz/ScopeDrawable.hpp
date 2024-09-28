#pragma once

#include <SFML/Graphics.hpp>

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

public:
	ScopeDrawable(const sf::IntRect &rect, const bool backwards = false)
		: rect{rect},
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
			shapes[i].setPosition({
				shapes[i].getPosition().x,
				std::clamp(half_height + (-half_height * audio[i]), 0.f, (float)rect.size.y),
			});
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &shape : shapes)
			target.draw(shape, states);
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
			// clang-format on

			shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});
		}
	}
};

} // namespace viz
