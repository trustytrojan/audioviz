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
};

} // namespace audioviz::fx
