#pragma once

#include <SFML/Graphics.hpp>

// not finished or tested yet
// in theory should work (need to change how audio is read in)

namespace viz
{

template <typename ShapeType>
class ScopeDrawable : public sf::Drawable
{
	// spectrum parameters
	float vert_multiplier = 20;

	// internal data
	sf::IntRect rect;
	bool backwards = false;

	struct
	{
		int width = 10, spacing = 5;
	} shape;

public:
	std::vector<ShapeType> shapes;

	void set_full_screen_spacing() { shape.spacing = rect.size.x / shapes.size(); }

	void set_shape_spacing(int _space)
	{
		shape.spacing = _space;
		update_shape_x_positions();
	}
	void set_shape_width(int _wid)
	{
		shape.width = _wid;
		update_shape_x_positions();
	}

	void set_multiplier(int mult) { vert_multiplier = mult; }

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_shape_x_positions();
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
			const auto middle = (rect.size.y / 2.f);
			shapes[i].setPosition({
				shapes[i].getPosition().x,
				std::clamp(middle + -vert_multiplier * audio[i], 0.f, (float)rect.size.y),
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
