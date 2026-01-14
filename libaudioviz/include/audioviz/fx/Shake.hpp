#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>

namespace audioviz::fx::Shake
{

void setParameters(AudioAnalyzer &aa, int sample_rate_hz, int fft_size, float multiplier);
const sf::Shader &getShader();

} // namespace audioviz::fx::Shake
