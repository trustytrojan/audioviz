#pragma once

#include <audioviz/fx/Effect.hpp>

namespace audioviz::fx
{

// Sets the alpha of the entire texture.
struct Alpha : Effect
{
	float alpha;
	Alpha(float alpha);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
