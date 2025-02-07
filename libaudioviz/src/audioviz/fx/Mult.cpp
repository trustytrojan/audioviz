#include <audioviz/fx/Mult.hpp>

namespace audioviz::fx
{

Mult::Mult(const float factor)
	: factor(factor)
{
}

void Mult::apply(RenderTexture &rt) const
{
	shader.setUniform("factor", factor);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace fx
