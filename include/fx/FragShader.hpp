#pragma once

#include <SFML/Graphics.hpp>

namespace fx
{

struct FragShader : sf::Shader
{
	FragShader(const char *file);
};

} // namespace fx
