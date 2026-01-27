#include "shader_headers/shake.vert.h"
#include <SFML/System/Clock.hpp>
#include <avz/gfx/fx/Shake.hpp>
#include <numbers>

static sf::Shader shader;
static sf::Clock _clock;

static void init()
{
	if (shader.getNativeHandle())
		return;
	if (!shader.loadFromMemory(std::string{libavz_shader_shake_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load shake shader!"};
}

namespace avz::fx
{

const sf::Shader &Shake::getShader() const
{
	return shader;
}

void Shake::setShaderUniforms() const
{
	init();
	shader.setUniform("time", _clock.getElapsedTime().asSeconds());
	shader.setUniform("frequencies", frequencies);
	shader.setUniform("amplitudes", amplitudes);
}

void Shake::setParameters(sf::Vector3f frequencies_hz, sf::Vector3f amplitudes, float multiplier)
{
	// Apply multiplier to amplitudes and convert frequencies to radians

	static const sf::Vector3f two_pi3{
		2 * std::numbers::pi_v<float>, 2 * std::numbers::pi_v<float>, 2 * std::numbers::pi_v<float>};
	const sf::Vector3f multiplier3{multiplier, multiplier, multiplier};

	frequencies = frequencies_hz.componentWiseMul(two_pi3);
	this->amplitudes = amplitudes.componentWiseMul(multiplier3);
}

} // namespace avz::fx
