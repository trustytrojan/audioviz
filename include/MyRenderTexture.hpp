#pragma once

#include <SFML/Graphics.hpp>

struct MyRenderTexture : public sf::RenderTexture
{
	const sf::Sprite sprite;

	MyRenderTexture(const sf::Vector2u size, const sf::ContextSettings &ctx = sf::ContextSettings())
		: sprite(getTexture(), {{}, (sf::Vector2i)size})
	{
		if (!create(size, ctx))
			throw std::runtime_error("failed to create render-texture!");
	}

	MyRenderTexture(const sf::Vector2u size, int antialiasing = 0)
		: MyRenderTexture(size, sf::ContextSettings{0, 0, antialiasing}) {}

	void copy(const MyRenderTexture &other)
	{
		draw(other.sprite);
		display();
	}
};