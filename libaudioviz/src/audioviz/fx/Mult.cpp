#include <audioviz/fx/Mult.hpp>

#include "shader_headers/mult.frag.h"

static sf::Shader shader;

namespace audioviz::fx
{

Mult::Mult(float factor)
	: factor{factor}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(audioviz_shader_mult_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load mult shader!"};
}

void Mult::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("factor", factor);
	rt.draw(rt, &shader);
	rt.display();
}

} // namespace audioviz::fx
