#pragma once

#include <avz/gfx/fx/TransformEffect.hpp>
#include <SFML/Graphics.hpp>

namespace avz::fx
{

struct Polar : TransformEffect
{
	sf::Vector2f size;
	float base_radius, max_radius;
	float angle_start;
	float angle_span;

	Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;
};

} // namespace avz::fx
