#pragma once

#include "audioviz/Sprite.hpp"
#include <SFML/Graphics.hpp>

namespace audioviz
{

/**
 * Extension of `sf::RenderTexture` that:
 * - Has a `sprite()` method to make the render-texture easily drawable.
 * - Adds a `copy` method to quickly `draw()` and `display()` another `RenderTexture` 's sprite.
 */
class RenderTexture : public sf::RenderTexture
{
public:
	RenderTexture(const sf::Vector2u size, int antialiasing = 0);

	// Copy the contents of `other` to this render-texture.
	void copy(const RenderTexture &other);

	inline Sprite sprite() const { return getTexture(); }
};

} // namespace audioviz
