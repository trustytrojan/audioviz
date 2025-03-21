#include <audioviz/fx/Blur.hpp>

namespace audioviz::fx
{

Blur::Blur(const float hrad, const float vrad, const int n_passes)
	: hrad(hrad),
	  vrad(vrad),
	  n_passes(n_passes)
{
}

void Blur::apply(RenderTexture &rt) const
{
	for (int i = 0; i < n_passes; ++i)
	{
		shader.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
		rt.draw(rt.sprite(), &shader);
		rt.display();

		shader.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
		rt.draw(rt.sprite(), &shader);
		rt.display();

		// the blur can point in any direction (hence the name of the uniform),
		// but anything other than horizontal/vertical will give you astigmatism.
		// experiment to your liking.
	}
}

} // namespace fx
