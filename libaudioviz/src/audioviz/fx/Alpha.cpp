#include <audioviz/fx/Alpha.hpp>

namespace audioviz::fx
{

Alpha::Alpha(float alpha)
	: alpha(alpha)
{
}

void Alpha::apply(RenderTexture &rt) const
{
	shader.setUniform("alpha", alpha);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace fx
