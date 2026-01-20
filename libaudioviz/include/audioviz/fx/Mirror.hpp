#pragma once

#include <audioviz/RenderTexture.hpp>
#include <audioviz/fx/PostProcessEffect.hpp>

namespace audioviz::fx
{

// Mirrors the texture horizontally based on configured side.
// mirror_side: 0 = mirror left side to right, 1 = mirror right side to left
class Mirror : public PostProcessEffect
{
	mutable RenderTexture rt2;

public:
	int mirror_side; // 0 or 1

	Mirror(int mirror_side = 0);

	void apply(RenderTexture &rt) const override;
	inline void setRtSize(sf::Vector2u size) override { rt2 = {size}; }
};

} // namespace audioviz::fx
