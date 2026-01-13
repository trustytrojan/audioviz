#include "shader_headers/polar.vert.h"
#include <audioviz/fx/Polar.hpp>
#include <stdexcept>
#include <string>

static sf::Shader shader;

static void init()
{
	if (shader.getNativeHandle())
		return;

	if (!shader.loadFromMemory(std::string{audioviz_shader_polar_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load polar shader!"};
}

namespace audioviz::fx::Polar
{

void setParameters(sf::Vector2f size, float base_radius, float max_radius)
{
	init();
	shader.setUniform("size", size);
	shader.setUniform("base_radius", base_radius);
	shader.setUniform("max_radius", max_radius);
}

const sf::Shader &getShader()
{
	init();
	return shader;
}

} // namespace audioviz::fx::Polar
