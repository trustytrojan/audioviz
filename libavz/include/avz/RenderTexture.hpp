#pragma once

#include "avz/Sprite.hpp"
#include <SFML/Graphics.hpp>

namespace audioviz
{

/**
 * Convenience extension of `sf::RenderTexture`
 */
class RenderTexture : public sf::RenderTexture
{
public:
	inline RenderTexture()
		: sf::RenderTexture{}
	{
	}

	inline RenderTexture(const sf::Vector2u size, unsigned antialiasing = 0)
		: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}}
	{
	}

	inline operator Sprite() const { return getTexture(); }
};

} // namespace audioviz
