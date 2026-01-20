#pragma once

#include <avz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

// Multiplies the entire texture's COLORS by a factor.
// Alphas are not affected.
struct Mult : public PostProcessEffect
{
	float factor;
	Mult(float factor);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
