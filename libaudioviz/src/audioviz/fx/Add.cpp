#include <audioviz/fx/Add.hpp>

static sf::Shader shader{std::filesystem::path{"shaders/add-110.frag"}, sf::Shader::Type::Fragment};

namespace audioviz::fx
{

Add::Add(float addend)
	: addend{addend}
{
}

void Add::apply(RenderTexture &rt) const
{
	shader.setUniform("size", sf::Glsl::Vec2{rt.getSize()});
	shader.setUniform("addend", addend);
	rt.draw(rt.sprite(), &shader);
	rt.display();
}

} // namespace audioviz::fx
