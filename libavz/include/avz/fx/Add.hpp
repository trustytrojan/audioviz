#pragma once

#include <avz/fx/PostProcessEffect.hpp>

namespace avz::fx
{

// Adds an addend to the entire texture's COLORS. Alphas are not affected.
struct Add : PostProcessEffect
{
	float addend;
	Add(float addend);
	void apply(RenderTexture &rt) const override;
};

} // namespace avz::fx
