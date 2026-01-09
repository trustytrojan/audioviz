#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>

namespace audioviz::fx::Shake
{

void setParameters(const AudioAnalyzer &aa, int sample_rate_hz, int fft_size, float multiplier);
void setParameters(sf::Vector2f amplitude, float frequency);
const sf::Shader &getShader();

} // namespace audioviz::fx
