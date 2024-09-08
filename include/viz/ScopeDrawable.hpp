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
	float multiplier = 4;

	// internal data
	std::vector<ShapeType> shapes;
	sf::IntRect rect;
	bool backwards = false;

	struct
	{
		int width = 10, spacing = 5;
	} shape;

public:
	ScopeDrawable()
	{
		for (auto &shape : shapes)
		{
			shape.setRadius(10);
			shape.setFillColor(sf::Color::White);
		}
	}

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		// update_shapes();
	}

	void update_shape_positions(const std::span<float> &audio)
	{
		if (shapes.size() != audio.size())
		{
			shapes.resize(audio.size());

			for (int i = 0; i < audio.size(); ++i)
			{
				// clang-format off
				const auto x = backwards
					? rect.position.x + rect.size.x - shape.width - i * (shape.width + shape.spacing)
					: rect.position.x + i * (shape.width + shape.spacing);
				// clang-format on

				shapes[i].setPosition({x, rect.position.y + rect.size.y / 2});
			}
		}

		for (int i = 0; i < (int)audio.size(); ++i)
		{
			const auto [x, y] = shapes[i].getPosition();
			shapes[i].setPosition({
				x,
				std::clamp(y + multiplier * rect.size.y * audio[i], 0.f, (float)rect.size.y),
			});
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &shape : shapes)
			target.draw(shape, states);
	}
};

} // namespace viz
