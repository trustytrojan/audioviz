#pragma once

#include <SFML/Graphics.hpp>

namespace fx
{

struct FragShader : sf::Shader
{
	FragShader(const char *const file)
	{
		if (!loadFromFile(file, sf::Shader::Type::Fragment))
			throw std::runtime_error("failed to load shader from file '" + std::string(file) + '\'');
	}
};

} // namespace fx
