#include <audioviz/fx/Blur.hpp>

#include "shader_headers/blur.frag.h"

static sf::Shader shader;

namespace audioviz::fx
{

Blur::Blur(float hrad, float vrad, int n_passes)
	: hrad{hrad},
	  vrad{vrad},
	  n_passes{n_passes}
{
	if (!shader.getNativeHandle() && !shader.loadFromMemory(audioviz_shader_blur_frag, sf::Shader::Type::Fragment))
		throw std::runtime_error{"failed to load blur shader!"};
}

void Blur::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	for (int i = 0; i < n_passes; ++i)
	{
		shader.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
		// maybe check rt2 for null? super rare that this would be a problem though
		rt2->clear();
		rt2->draw(rt.sprite(), &shader);
		rt2->display();
		rt.draw(rt2->sprite());
		rt.display();

		shader.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
		rt2->clear();
		rt2->draw(rt.sprite(), &shader);
		rt2->display();
		rt.draw(rt2->sprite());
		rt.display();

		// the blur can point in any direction (hence the name of the uniform),
		// but anything other than horizontal/vertical will give you astigmatism.
		// experiment to your liking.
	}
}

void Blur::setRtSize(const sf::Vector2u size)
{
	if (!size.x || !size.y)
		rt2 = {};
	rt2 = std::make_unique<RenderTexture>(size);
}

} // namespace audioviz::fx
