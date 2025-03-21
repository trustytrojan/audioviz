#include <audioviz/fx/Mult.hpp>

static sf::Shader shader{std::filesystem::path{"shaders/mult-110.frag"}, sf::Shader::Type::Fragment};

namespace audioviz::fx
{

Mult::Mult(float factor)
	: factor{factor}
{
}

void Mult::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("factor", factor);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace audioviz::fx
