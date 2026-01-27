#pragma once

#include <avz/gfx/RenderTexture.hpp>

namespace avz::fx
{

class PostProcessEffect
{
public:
	virtual ~PostProcessEffect() = default;

	// Apply this post-process effect onto a render-texture.
	virtual void apply(RenderTexture &) const = 0;

	// Sets the internal render-texture size of the effect, if its implementation has one.
	virtual void setRtSize(sf::Vector2u) {}
};

} // namespace avz::fx
