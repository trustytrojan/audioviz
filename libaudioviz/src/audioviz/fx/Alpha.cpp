#include <audioviz/fx/Alpha.hpp>

#include "shader_headers/alpha.frag.h"

static sf::Shader shader;

namespace audioviz::fx
{

Alpha::Alpha(float alpha)
	: alpha{alpha}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(audioviz_shader_alpha_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load add shader!"};
}

void Alpha::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("alpha", alpha);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace audioviz::fx
