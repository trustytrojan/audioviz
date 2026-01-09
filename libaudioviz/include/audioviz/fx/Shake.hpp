#pragma once

#include "SpaceEffect.hpp"

namespace audioviz::fx
{

// Applies a screen-space shake/jitter by offsetting sprite vertices.
struct Shake : SpaceEffect
{
	sf::Vector2f amplitude; // in pixels
	float frequency;        // oscillations per second-ish

	Shake(sf::Vector2f amplitude, float frequency = 20.f);
	void apply(sf::RenderTarget &, const sf::Drawable &) const override;
};

} // namespace audioviz::fx
