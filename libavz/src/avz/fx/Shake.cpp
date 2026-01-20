#include "shader_headers/shake.vert.h"
#include <SFML/System/Clock.hpp>
#include <avz/fx/Shake.hpp>
#include <avz/util.hpp>
#include <numbers>

static sf::Shader shader;
static sf::Clock _clock;

static void init()
{
	if (shader.getNativeHandle())
		return;
	if (!shader.loadFromMemory(std::string{audioviz_shader_shake_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load shake shader!"};
}

namespace audioviz::fx
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

void Shake::setParameters(AudioAnalyzer &aa, int from_hz, int to_hz, float multiplier)
{
	auto bands = aa.compute_multiband_shake(from_hz, to_hz);

	// Apply multiplier to amplitudes and convert frequencies to radians
	frequencies.x = bands[0].frequency_hz * 2.f * std::numbers::pi_v<float>;
	frequencies.y = bands[1].frequency_hz * 2.f * std::numbers::pi_v<float>;
	frequencies.z = bands[2].frequency_hz * 2.f * std::numbers::pi_v<float>;

	amplitudes.x = bands[0].amplitude * multiplier;
	amplitudes.y = bands[1].amplitude * multiplier;
	amplitudes.z = bands[2].amplitude * multiplier;
}

} // namespace audioviz::fx
