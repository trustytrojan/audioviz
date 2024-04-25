#pragma once

#include <SFML/Graphics.hpp>

class MyRenderTexture : public sf::RenderTexture
{
	static inline sf::Shader _blur, _mult;
	static inline bool _shaders_loaded = false;

	static void load_frag_shader(sf::Shader &shader, const char *const file)
	{
		if (!shader.loadFromFile(file, sf::Shader::Type::Fragment))
			throw std::runtime_error("failed to load shader from file '" + std::string(file) + '\'');
	}

public:
	sf::Sprite sprite;

	MyRenderTexture(const sf::Vector2u &size, const sf::ContextSettings &settings = sf::ContextSettings())
		: sprite(getTexture(), {{}, (sf::Vector2i)size})
	{
		if (!_shaders_loaded)
		{
			load_frag_shader(_blur, "shaders/blur.frag");
			load_frag_shader(_mult, "shaders/mult.frag");
			_shaders_loaded = true;
		}

		if (!create(size, settings))
			throw std::runtime_error("failed to create render-texture!");
	}

	void blur(float hrad, float vrad, int npasses)
	{
		for (int i = 0; i < npasses; ++i)
		{
			_blur.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
			draw(sprite, &_blur);
			display();

			_blur.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
			draw(sprite, &_blur);
			display();
		}
	}

	void multiply(float factor)
	{
		_mult.setUniform("factor", factor);
		draw(sprite, &_mult);
		display();
	}
};
