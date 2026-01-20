#include <audioviz/fx/Mirror.hpp>

#include "shader_headers/mirror.frag.h"

static sf::Shader shader;

namespace audioviz::fx
{

Mirror::Mirror(int mirror_side)
	: mirror_side{mirror_side}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(audioviz_shader_mirror_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load mirror shader!"};
}

void Mirror::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("mirror_side", mirror_side);
	rt2.clear(sf::Color::Transparent);
	rt2.draw(rt, &shader);
	rt2.display();

	rt.draw(rt2, &shader);
	rt.display();
}

} // namespace audioviz::fx
