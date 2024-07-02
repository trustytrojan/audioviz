#pragma once

#include "FragShader.hpp"
#include "RenderTexture.hpp"

namespace fx
{

// Multiplies the entire texture by a factor.
class Add : public Effect
{
	static inline FragShader shader = "shaders/add.frag";

public:
	float addend;

	Add(float addend) : addend(addend) {}

	void apply(RenderTexture &rt) const override
	{
		shader.setUniform("addend", addend);
		rt.draw(rt.sprite, &shader);
		rt.display();
	}
};

} // namespace fx
