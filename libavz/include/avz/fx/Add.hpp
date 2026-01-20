#pragma once

#include <avz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

// Adds an addend to the entire texture's COLORS. Alphas are not affected.
struct Add : PostProcessEffect
{
	float addend;
	Add(float addend);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
