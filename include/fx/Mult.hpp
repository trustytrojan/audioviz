#pragma once

#include "FragShader.hpp"
#include "Effect.hpp"

namespace fx
{

// Multiplies the entire texture by a factor.
class Mult : public Effect
{
	static inline FragShader shader = "shaders/mult.frag";

public:
	float factor;

	Mult(float factor);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
