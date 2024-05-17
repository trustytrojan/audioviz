#pragma once

#include "Effect.hpp"
#include "FragShader.hpp"
#include "RenderTexture.hpp"

namespace fx
{

/**
 * Perform a Gaussian blur with linear-sampling.
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

	Blur(float hrad, float vrad, int n_passes)
		: hrad(hrad), vrad(vrad), n_passes(n_passes) {}

	void apply(RenderTexture &rt) const override
	{
		for (int i = 0; i < n_passes; ++i)
		{
			shader.setUniform("direction", sf::Glsl::Vec2{hrad, 0}); // horizontal blur
			rt.draw(rt.sprite, &shader);
			rt.display();

			shader.setUniform("direction", sf::Glsl::Vec2{0, vrad}); // vertical blur
			rt.draw(rt.sprite, &shader);
			rt.display();

			// the blur can point in any direction (hence the name of the uniform),
			// but anything other than horizontal/vertical will give you astigmatism.
			// experiment to your liking.
		}
	}
};

} // namespace fx
