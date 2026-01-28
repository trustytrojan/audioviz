#include "shader_headers/polar.vert.h"
#include "shader_headers/spectrum_polar.geom.h"
#include "shader_headers/spectrum_polar_lines.geom.h"
#include <avz/gfx/fx/Polar.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

static sf::Shader shader, shader_gs, shader_gs_lines;

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
	if (shader_gs.getNativeHandle() && shader_gs_lines.getNativeHandle())
		return;

	// Pass-through vertex shader to satisfy the linker
	static const std::string vs_src =
		"#version 120\nvoid main() { gl_Position = gl_Vertex; gl_FrontColor = gl_Color; }";
	static const std::string fs_src = "#version 120\nvoid main() { gl_FragColor = gl_Color; }";

	if (!shader_gs.loadFromMemory(vs_src, std::string{libavz_shader_spectrum_polar_geom}, fs_src))
		throw std::runtime_error{"failed to load spectrum_polar GS shader!"};

	if (!shader_gs_lines.loadFromMemory(vs_src, std::string{libavz_shader_spectrum_polar_lines_geom}, fs_src))
		throw std::runtime_error{"failed to load spectrum_polar_lines GS shader!"};
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
	if (use_gs_spectrum_bars)
	{
		init_gs();
		return (spectrum_bar_width == 1) ? shader_gs_lines : shader_gs;
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

	if (use_gs_spectrum_bars)
	{
		if (spectrum_bar_width > 1)
			s.setUniform("bar_width", spectrum_bar_width);
		s.setUniform("bottom_y", spectrum_bottom_y);
	}
}

} // namespace avz::fx
