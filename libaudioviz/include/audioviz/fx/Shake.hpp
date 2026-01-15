#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>

namespace audioviz::fx::Shake
{

void setParameters(AudioAnalyzer &aa, int from_hz, int to_hz, float multiplier);
const sf::Shader &getShader();

} // namespace audioviz::fx::Shake
