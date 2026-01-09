#include "shader_headers/shake.vert.h"
#include <SFML/System/Clock.hpp>
#include <audioviz/fx/Shake.hpp>
#include <audioviz/util.hpp>
#include <print>

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

void setParameters(const AudioAnalyzer &aa, int sample_rate_hz, int fft_size, float multiplier)
{
	std::println("setParameters: sample_rate_hz={} fft_size={}", sample_rate_hz, fft_size);

	float amp_sum = 0.f;
	float hz_sum = 0.f;

	for (int ch = 0; ch < aa.get_num_channels(); ++ch)
	{
		const auto &spec = aa.get_spectrum_data(ch);
		const auto idx = util::weighted_max_index(spec, expf, 100.f);
		std::println("idx={} val={}", idx, spec[idx]);
		amp_sum += spec[idx];
		hz_sum += (static_cast<float>(idx) * sample_rate_hz) / static_cast<float>(fft_size);
	}

	std::println("hz_sum={}", hz_sum);

	const float amp_avg = amp_sum / aa.get_num_channels();
	const float hz_avg = hz_sum / aa.get_num_channels();

	std::println("hz_avg={}", hz_avg);

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
