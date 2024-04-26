#pragma once

#include <SFML/Graphics.hpp>

class MyRenderTexture : public sf::RenderTexture
{
	static inline sf::Shader s_blur, s_mult;
	static inline bool _shaders_loaded = false;

	static void load_frag_shader(sf::Shader &shader, const char *const file)
	{
		if (!shader.loadFromFile(file, sf::Shader::Type::Fragment))
			throw std::runtime_error("failed to load shader from file '" + std::string(file) + '\'');
	}

	sf::Sprite m_sprite;

public:
	MyRenderTexture(const sf::Vector2u &size, const sf::ContextSettings &settings = sf::ContextSettings())
		: m_sprite(getTexture(), {{}, (sf::Vector2i)size})
	{
		// lazy initialization
		if (!_shaders_loaded)
		{
			load_frag_shader(s_blur, "shaders/blur.frag");
			load_frag_shader(s_mult, "shaders/mult.frag");
			_shaders_loaded = true;
		}

		if (!create(size, settings))
			throw std::runtime_error("failed to create render-texture!");
	}

	const sf::Sprite &sprite()
	{
		return m_sprite;
	}

	/**
	 * Perform a Gaussian blur with linear-sampling.
	 * This method executes the shader with horizontal and vertical blurs `n_passes` times.
	 * `hrad` and `vrad` are the horizontal and vertical blur radii, respectively.
	 */
	void blur(float hrad, float vrad, int n_passes)
	{
		for (int i = 0; i < n_passes; ++i)
		{
			s_blur.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
			draw(m_sprite, &s_blur);
			display();

			s_blur.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
			draw(m_sprite, &s_blur);
			display();

			// the blur can point in any direction (hence the name of the uniform),
			// but anything other than up/down behaves weirdly.
			// experiment to your liking.
		}
	}

	/**
	 * Multiply every pixel by `factor`.
	 */
	void mult(float factor)
	{
		s_mult.setUniform("factor", factor);
		draw(m_sprite, &s_mult);
		display();
	}
};
