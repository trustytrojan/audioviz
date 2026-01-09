#pragma once

#include <audioviz/RenderTexture.hpp>

namespace audioviz::fx
{

class SpaceEffect
{
public:
	virtual ~SpaceEffect() = default;

	// Apply this effect onto a drawable being drawn to a target.
	virtual void apply(sf::RenderTarget &, const sf::Drawable &) const = 0;
};

} // namespace audioviz::fx
