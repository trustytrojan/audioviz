#pragma once

#include "FragShader.hpp"
#include "Effect.hpp"

namespace fx
{

/**
 * Performs a Gaussian blur with linear-sampling.
 * This method executes the shader with horizontal and vertical blurs `n_passes` times.
 * `hrad` and `vrad` are the horizontal and vertical blur radii, respectively.
 * It is recommended to use a zero-alpha background if this is being used to create a glow.
 */
class Blur : public Effect
{
	static inline FragShader shader = "shaders/blur.frag";

public:
	float hrad, vrad;
	int n_passes;

	Blur(float hrad, float vrad, int n_passes);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
