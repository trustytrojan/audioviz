#pragma once

#include <audioviz/RenderTexture.hpp>
#include <audioviz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

/**
 * Performs a Gaussian blur with linear-sampling.
 * This method executes the shader with horizontal and vertical blurs `n_passes` times.
 * `hrad` and `vrad` are the horizontal and vertical blur radii, respectively.
 * It is recommended to use a zero-alpha background if this is being used to create a glow.
 */
struct Blur : PostProcessEffect
{
	float hrad, vrad;
	int n_passes;
	std::unique_ptr<RenderTexture> rt2;
	Blur(float hrad, float vrad, int n_passes);
	void apply(RenderTexture &rt) const override;
	void setRtSize(sf::Vector2u) override;
};

} // namespace audioviz::fx
