#pragma once

#include "tt/RenderTexture.hpp"

namespace fx
{

class Effect
{
public:
	// Apply this effect onto a render-texture.
	virtual void apply(tt::RenderTexture &) const = 0;
};

} // namespace fx
