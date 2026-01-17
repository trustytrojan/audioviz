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

namespace audioviz::fx
{

Polar::Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span)
	: size{size},
	  base_radius{br},
	  max_radius{mr},
	  angle_start{angle_start},
	  angle_span{angle_span}
{
}

const sf::Shader &Polar::getShader() const
{
	init();
	return shader;
}

void Polar::setShaderUniforms() const
{
	init();
	shader.setUniform("size", size);
	shader.setUniform("base_radius", base_radius);
	shader.setUniform("max_radius", max_radius);
	shader.setUniform("angle_start", angle_start);
	shader.setUniform("angle_span", angle_span);
}

} // namespace audioviz::fx
