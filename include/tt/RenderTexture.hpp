#pragma once

#include <SFML/Graphics.hpp>

namespace tt {

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
