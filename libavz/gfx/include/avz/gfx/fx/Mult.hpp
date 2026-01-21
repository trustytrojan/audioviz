#pragma once

#include <avz/gfx/fx/PostProcessEffect.hpp>

namespace avz::fx
{

// Multiplies the entire texture's COLORS by a factor.
// Alphas are not affected.
struct Mult : public PostProcessEffect
{
	float factor;
	Mult(float factor);
	void apply(RenderTexture &rt) const override;
};

} // namespace avz::fx
