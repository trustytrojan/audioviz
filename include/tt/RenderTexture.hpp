#pragma once

#include <SFML/Graphics.hpp>

namespace tt {

/**
 * Subclass of `sf::RenderTexture` that is only meant to be created *once*.
 * Has a public `const sf::Sprite` member to make the render-texture easily drawable.
 * (Inheriting from `sf::Drawable` would cause a diamond-inheritance problem, so this was the better solution).
 * Adds a `copy` method to quickly draw its contents onto another `tt::RenderTexture`.
 */
struct RenderTexture : public sf::RenderTexture
{
	const sf::Sprite sprite;

	RenderTexture(const sf::Vector2u size, const sf::ContextSettings &ctx = sf::ContextSettings())
		: sprite(getTexture(), {{}, (sf::Vector2i)size})
	{
		if (!create(size, ctx))
			throw std::runtime_error("failed to create render-texture!");
	}

	RenderTexture(const sf::Vector2u size, int antialiasing = 0)
		: RenderTexture(size, sf::ContextSettings{0, 0, antialiasing}) {}

	void copy(const RenderTexture &other)
	{
		draw(other.sprite);
		display();
	}
};

} // namespace tt
