#include "fx/Alpha.hpp"

namespace fx
{

Alpha::Alpha(float alpha)
	: alpha(alpha)
{
}

void Alpha::apply(tt::RenderTexture &rt) const
{
	shader.setUniform("alpha", alpha);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace fx
