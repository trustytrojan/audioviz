#include "fx/Mult.hpp"

namespace fx
{

Mult::Mult(const float factor)
	: factor(factor)
{
}

void Mult::apply(tt::RenderTexture &rt) const
{
	shader.setUniform("factor", factor);
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	rt.draw(rt.sprite, &shader);
	rt.display();
}

} // namespace fx
