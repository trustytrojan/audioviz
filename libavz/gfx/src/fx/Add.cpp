#include <avz/gfx/fx/Add.hpp>

#include "shader_headers/add.frag.h"

static sf::Shader shader;

namespace avz::fx
{

Add::Add(float addend)
	: addend{addend}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(libavz_shader_add_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load add shader!"};
}

void Add::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("addend", addend);
	rt.draw(rt, &shader);
	rt.display();
}

} // namespace avz::fx
