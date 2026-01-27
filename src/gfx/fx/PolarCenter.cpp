#include "shader_headers/polar_center.vert.h"
#include <avz/gfx/fx/PolarCenter.hpp>
#include <stdexcept>
#include <string>

static sf::Shader shader;

static void init()
{
	if (shader.getNativeHandle())
		return;
	if (!shader.loadFromMemory(std::string{libavz_shader_polar_center_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load polar_center shader!"};
}

namespace avz::fx
{

PolarCenter::PolarCenter(sf::Vector2f size, float br, float mr, float angle_start, float angle_span)
	: size{size},
	  base_radius{br},
	  max_radius{mr},
	  angle_start{angle_start},
	  angle_span{angle_span}
{
}

const sf::Shader &PolarCenter::getShader() const
{
	init();
	return shader;
}

void PolarCenter::setShaderUniforms() const
{
	init();
	shader.setUniform("size", size);
	shader.setUniform("base_radius", base_radius);
	shader.setUniform("max_radius", max_radius);
	shader.setUniform("angle_start", angle_start);
	shader.setUniform("angle_span", angle_span);
}

} // namespace avz::fx
