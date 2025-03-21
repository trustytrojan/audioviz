#include <audioviz/fx/Alpha.hpp>

static sf::Shader shader{std::filesystem::path{"shaders/alpha.frag"}, sf::Shader::Type::Fragment};

namespace audioviz::fx
{

Alpha::Alpha(float alpha)
	: alpha{alpha}
{
}

void Alpha::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("alpha", alpha);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace audioviz::fx
