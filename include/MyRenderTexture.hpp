#pragma once

#include <SFML/Graphics.hpp>

struct MyRenderTexture : sf::RenderTexture
{
	sf::Sprite sprite;

	MyRenderTexture(const sf::Vector2u &size, const sf::ContextSettings &settings = sf::ContextSettings())
		: sprite(getTexture(), {{}, (sf::Vector2i)size})
	{
		if (!create(size, settings))
			throw std::runtime_error("failed to create render-texture!");
	}

	void blur(sf::Shader &shader, float hrad, float vrad, int npasses)
	{
		for (int i = 0; i < npasses; ++i)
		{
			shader.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
			draw(sprite, &shader);
			display();

			shader.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
			draw(sprite, &shader);
			display();
		}
	}
};
