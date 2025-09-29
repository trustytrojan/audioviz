#pragma once

#include <audioviz/RenderTexture.hpp>

namespace audioviz::fx
{

class Effect
{
public:
	virtual ~Effect() = default;

	// Apply this effect onto a render-texture.
	virtual void apply(RenderTexture &) const = 0;

	// Sets the internal render-texture size of the effect, if its implementation has one.
	virtual void setRtSize(sf::Vector2u) {}
};

} // namespace audioviz::fx
