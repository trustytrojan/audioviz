#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/fx/TransformEffect.hpp>

namespace avz::fx
{

struct Shake : TransformEffect
{
	sf::Vector3f frequencies, amplitudes;

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;

	void setParameters(sf::Vector3f frequencies, sf::Vector3f amplitudes, float multiplier);
};

} // namespace avz::fx
