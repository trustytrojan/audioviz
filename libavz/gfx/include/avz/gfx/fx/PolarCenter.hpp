#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/fx/TransformEffect.hpp>

namespace avz::fx
{

struct PolarCenter : TransformEffect
{
	sf::Vector2f size;
	float base_radius, max_radius;
	float angle_start;
	float angle_span;

	PolarCenter(sf::Vector2f size, float br, float mr, float angle_start, float angle_span);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;
};

} // namespace avz::fx
