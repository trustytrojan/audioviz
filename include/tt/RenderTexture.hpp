#pragma once

#include <SFML/Graphics.hpp>

namespace tt
{

/**
 * Subclass of `sf::RenderTexture`.
 * Has a public `const sf::Sprite` member to make the render-texture easily drawable.
 * Adds a `copy` method to quickly `draw()` and `display()` another `tt::RenderTexture` 's sprite.
 */
class RenderTexture : public sf::RenderTexture
{
public:
	RenderTexture(const sf::Vector2u size, int antialiasing = 0)
		: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}}
	{
	}

	sf::Sprite sprite() const { return sf::Sprite{getTexture()}; }

	// Copy the contents of `other` to this render-texture.
	void copy(const RenderTexture &other)
	{
		draw(other.sprite());
		display();
	}
};

} // namespace tt
