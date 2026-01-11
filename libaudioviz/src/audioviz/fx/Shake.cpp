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

void setParameters(AudioAnalyzer &aa, int sample_rate_hz, int fft_size, float multiplier)
{
	aa.compute_peak_freq_amp(sample_rate_hz, fft_size, 250);
	float amp_avg{}, hz_avg{};
	for (int ch = 0; ch < aa.num_channels(); ++ch)
	{
		const auto &ch_data = aa.get_channel_data(ch);
		amp_avg += ch_data.peak_amplitude;
		hz_avg += ch_data.peak_frequency_hz;
	}
	amp_avg /= aa.num_channels();
	hz_avg /= aa.num_channels();
	const float amp = amp_avg * multiplier;
	const auto frequency = 2.f * std::numbers::pi_v<float> * hz_avg; // convert Hz -> rad/s for sin()
	setParameters({amp, amp}, frequency);
}

void setParameters(sf::Vector2f amplitude, float frequency)
{
	init();
	shader.setUniform("time", _clock.getElapsedTime().asSeconds());
	shader.setUniform("frequency", frequency);
	shader.setUniform("amplitude", amplitude);
}

const sf::Shader &getShader()
{
	init();
	return shader;
}

} // namespace audioviz::fx::Shake
