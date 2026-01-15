#include "shader_headers/shake.vert.h"
#include <SFML/System/Clock.hpp>
#include <audioviz/fx/Shake.hpp>
#include <audioviz/util.hpp>

static sf::Shader shader;
static sf::Clock _clock;

static void init()
{
	if (!shader.getNativeHandle() &&
		!shader.loadFromMemory(std::string{audioviz_shader_shake_vert}, sf::Shader::Type::Vertex))
		throw std::runtime_error{"failed to load shake shader!"};
}

namespace audioviz::fx::Shake
{

void setParameters(AudioAnalyzer &aa, int from_hz, int to_hz, float multiplier)
{
	auto bands = aa.compute_multiband_shake(from_hz, to_hz);

	// Apply multiplier to amplitudes and convert frequencies to radians
	sf::Vector3f frequencies, amplitudes;
	frequencies.x = bands[0].frequency_hz * 2.f * M_PI;
	frequencies.y = bands[1].frequency_hz * 2.f * M_PI;
	frequencies.z = bands[2].frequency_hz * 2.f * M_PI;

	amplitudes.x = bands[0].amplitude * multiplier;
	amplitudes.y = bands[1].amplitude * multiplier;
	amplitudes.z = bands[2].amplitude * multiplier;

	init();
	shader.setUniform("time", _clock.getElapsedTime().asSeconds());
	shader.setUniform("frequencies", frequencies);
	shader.setUniform("amplitudes", amplitudes);
}

const sf::Shader &getShader()
{
	init();
	return shader;
}

} // namespace audioviz::fx::Shake
