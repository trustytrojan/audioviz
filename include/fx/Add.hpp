#pragma once

#include "FragShader.hpp"
#include "Effect.hpp"

namespace fx
{

// Multiplies the entire texture by a factor.
class Add : public Effect
{
	static inline FragShader shader = "shaders/add.frag";

public:
	float addend;

	Add(float addend);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
