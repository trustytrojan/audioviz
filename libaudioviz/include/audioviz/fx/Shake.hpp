#pragma once

#include <audioviz/fx/Effect.hpp>

namespace audioviz::fx
{

// Applies a screen-space shake/jitter by offsetting sprite vertices.
struct Shake : Effect
{
	sf::Vector2f amplitude; // in pixels
	float frequency;        // oscillations per second-ish

	Shake(sf::Vector2f amplitude, float frequency = 20.f);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
