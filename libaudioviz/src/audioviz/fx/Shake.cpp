#include <audioviz/fx/Shake.hpp>

#include "shader_headers/shake.vert.h"

#include <SFML/System/Clock.hpp>

static sf::Shader shader;
static sf::Clock _clock;

namespace audioviz::fx
{

Shake::Shake(const sf::Vector2f amplitude, const float frequency)
	: amplitude{amplitude},
	  frequency{frequency}
{
	if (!shader.getNativeHandle() &&
		!shader.loadFromMemory(std::string{audioviz_shader_shake_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load shake shader!"};
}

void Shake::apply(sf::RenderTarget &target, const sf::Drawable &drawable) const
{
	shader.setUniform("time", _clock.getElapsedTime().asSeconds());
	shader.setUniform("frequency", frequency);
	shader.setUniform("amplitude", sf::Glsl::Vec2{amplitude});

	target.draw(drawable, &shader);
}

} // namespace audioviz::fx
