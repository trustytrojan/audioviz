#pragma once

#include "TransformEffect.hpp"
#include <SFML/Graphics.hpp>
#include <avz/fft/AudioAnalyzer.hpp>

namespace audioviz::fx
{

struct Shake : TransformEffect
{
	sf::Vector3f frequencies, amplitudes;

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;

	void setParameters(AudioAnalyzer &aa, int from_hz, int to_hz, float multiplier);
};

} // namespace audioviz::fx
