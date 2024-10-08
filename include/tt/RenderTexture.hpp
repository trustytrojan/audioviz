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
	sf::Sprite sprite;

	RenderTexture(const sf::Vector2u size, int antialiasing = 0)
		: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}},
		  sprite{getTexture()}
	{
	}

	// for some reason the texture that `sf::RenderTexture`
	// starts with gets replaced after the first `display()` call.
	// so to guarantee safety i need to do this...
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
