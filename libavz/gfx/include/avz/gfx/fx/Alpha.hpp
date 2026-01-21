#pragma once

#include <avz/gfx/fx/PostProcessEffect.hpp>

namespace avz::fx
{

// Sets the alpha of the entire texture.
struct Alpha : PostProcessEffect
{
	float alpha;
	Alpha(float alpha);
	void apply(RenderTexture &rt) const override;
};

} // namespace avz::fx
