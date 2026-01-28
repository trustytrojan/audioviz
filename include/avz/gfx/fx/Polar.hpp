#pragma once

#include <SFML/Graphics.hpp>
#include <avz/gfx/fx/TransformEffect.hpp>

namespace avz::fx
{

struct Polar : TransformEffect
{
	sf::Vector2f size;
	float base_radius, max_radius;
	float angle_start;
	float angle_span;
	float warping_factor;

	Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span, float warping_factor = 1.0f);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;
};

} // namespace avz::fx
