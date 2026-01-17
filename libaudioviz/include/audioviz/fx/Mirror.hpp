#pragma once

#include <audioviz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

// Mirrors the texture horizontally based on configured side.
// mirror_side: 0 = mirror right side to left, 1 = mirror left side to right
struct Mirror : PostProcessEffect
{
	int mirror_side; // 0 or 1

	Mirror(int mirror_side = 0);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
