#include "fx/Add.hpp"

namespace fx
{

Add::Add(float addend)
	: addend(addend)
{
}

void Add::apply(tt::RenderTexture &rt) const
{
	shader.setUniform("addend", addend);
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	rt.draw(rt.sprite, &shader);
	rt.display();
}

} // namespace fx
