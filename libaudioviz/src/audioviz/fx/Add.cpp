#include <audioviz/fx/Add.hpp>

#include "shader_headers/add.frag.h"

static sf::Shader shader;

namespace audioviz::fx
{

Add::Add(float addend)
	: addend{addend}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(audioviz_shader_add_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load add shader!"};
}

void Add::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("addend", addend);
	rt.draw(rt, &shader);
	rt.display();
}

} // namespace audioviz::fx
