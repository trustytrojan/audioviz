#include "shader_headers/polar.vert.h"
#include "shader_headers/polar_expansion.geom.h"
#include <avz/gfx/fx/Polar.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

static sf::Shader shader, shader_gs;

static void init()
{
	if (shader.getNativeHandle())
		return;

	static const std::string fs_src = "#version 120\nvoid main() { gl_FragColor = gl_Color; }";

	if (!shader.loadFromMemory(std::string{libavz_shader_polar_vert}, fs_src))
		throw std::runtime_error{"failed to load polar shader!"};
}

static void init_gs()
{
	if (shader_gs.getNativeHandle())
		return;

	// Pass-through vertex shader to satisfy the linker
	static const std::string vs_src = "#version 120\nvoid main() { gl_Position = gl_Vertex; gl_FrontColor = gl_Color; }";
	static const std::string fs_src = "#version 120\nvoid main() { gl_FragColor = gl_Color; }";

	if (!shader_gs.loadFromMemory(vs_src, std::string{libavz_shader_polar_expansion_geom}, fs_src))
		throw std::runtime_error{"failed to load polar_expansion GS shader!"};
}

namespace avz::fx
{

Polar::Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span, float warping_factor)
	: size{size},
	  base_radius{br},
	  max_radius{mr},
	  angle_start{angle_start},
	  angle_span{angle_span},
	  warping_factor{warping_factor}
{
}

const sf::Shader &Polar::getShader() const
{
	if (use_gs_expansion)
	{
		init_gs();
		return shader_gs;
	}
	init();
	return shader;
}

void Polar::setShaderUniforms() const
{
	sf::Shader &s = const_cast<sf::Shader &>(getShader());

	s.setUniform("size", size);
	s.setUniform("base_radius", base_radius);
	s.setUniform("max_radius", max_radius);
	s.setUniform("angle_start", angle_start);
	s.setUniform("angle_span", angle_span);
	s.setUniform("warping_factor", warping_factor);

	if (use_gs_expansion)
	{
		s.setUniform("bar_width", bar_width);
		s.setUniform("bottom_y", bottom_y);
	}
}

} // namespace avz::fx
