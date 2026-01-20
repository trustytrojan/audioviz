#pragma once

#include <avz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

// Sets the alpha of the entire texture.
struct Alpha : PostProcessEffect
{
	float alpha;
	Alpha(float alpha);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
