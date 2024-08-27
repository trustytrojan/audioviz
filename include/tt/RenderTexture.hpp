#pragma once

#include <SFML/Graphics.hpp>

namespace tt
{

/**
 * Subclass of `sf::RenderTexture`.
 * Has a public `const sf::Sprite` member to make the render-texture easily drawable.
 * Adds a `copy` method to quickly draw another `tt::RenderTexture`'s contents onto its.
 */
class RenderTexture : public sf::RenderTexture
{
public:
	sf::Sprite sprite;

	RenderTexture(const sf::Vector2u size, int antialiasing = 0)
		: sf::RenderTexture(size, {.antialiasingLevel = antialiasing}),
		  sprite(getTexture(), {{}, static_cast<sf::Vector2i>(size)})
	{
	}

	void display()
	{
		sf::RenderTexture::display();
		sprite.setTexture(getTexture(), true);
	}

	// Copy the contents of `other` to this render-texture.
	void copy(const RenderTexture &other)
	{
		draw(other.sprite);
		display();
	}
};

} // namespace tt
