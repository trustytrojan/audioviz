#include <audioviz/fx/Add.hpp>

namespace audioviz::fx
{

Add::Add(float addend)
	: addend(addend)
{
}

void Add::apply(RenderTexture &rt) const
{
	shader.setUniform("addend", addend);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace fx
