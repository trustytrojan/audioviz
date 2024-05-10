#pragma once

#include "RenderTexture.hpp"
#include "FragShader.hpp"

namespace fx
{
	// Multiplies the entire texture by a factor.
	class Mult : public Effect
	{
		static inline FragShader shader = "shaders/mult.frag";

	public:
		float factor;

		void apply(RenderTexture &rt) const override
		{
			shader.setUniform("factor", factor);
			rt.draw(rt.sprite, &shader);
			rt.display();
		}
	};
}
