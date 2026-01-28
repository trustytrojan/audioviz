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

	// Geometry shader expansion parameters (only for spectrum-bar expansion)
	bool use_gs_spectrum_bars{false};
	float spectrum_bar_width{0};
	float spectrum_bottom_y{0};

	Polar(sf::Vector2f size, float br, float mr, float angle_start, float angle_span, float warping_factor = 1.0f);

	virtual const sf::Shader &getShader() const override;
	virtual void setShaderUniforms() const override;

	inline void set_gs_spectrum_bars(bool enabled, float width, float bottom)
	{
		use_gs_spectrum_bars = enabled;
		spectrum_bar_width = width;
		spectrum_bottom_y = bottom;
	}
};

} // namespace avz::fx
