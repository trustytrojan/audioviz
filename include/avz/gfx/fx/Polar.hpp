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

	// Geometry shader expansion parameters
	bool use_gs_expansion{false};
	float bar_width{0};
	float bottom_y{0};

	Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span, float warping_factor = 1.0f);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;

	inline void set_gs_expansion(bool enabled, float width, float bottom)
	{
		use_gs_expansion = enabled;
		bar_width = width;
		bottom_y = bottom;
	}
};

} // namespace avz::fx
