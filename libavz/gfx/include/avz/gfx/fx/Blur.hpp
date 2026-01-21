#pragma once

#include <avz/gfx/RenderTexture.hpp>
#include "PostProcessEffect.hpp"

namespace avz::fx
{

/**
 * Performs a Gaussian blur with linear-sampling.
 * This method executes the shader with horizontal and vertical blurs `n_passes` times.
 * `hrad` and `vrad` are the horizontal and vertical blur radii, respectively.
 * It is recommended to use a zero-alpha background if this is being used to create a glow.
 */
class Blur : PostProcessEffect
{
	mutable RenderTexture rt2;

public:
	float hrad, vrad;
	int n_passes;

	Blur(float hrad, float vrad, int n_passes);

	void apply(RenderTexture &rt) const override;
	inline void setRtSize(sf::Vector2u size) override { rt2 = {size}; }
};

} // namespace avz::fx
