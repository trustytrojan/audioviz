#pragma once

#include <SFML/Graphics.hpp>

namespace fx
{
	struct RenderTexture;

	class Effect
	{
	public:
		virtual void apply(RenderTexture &) const = 0;
	};
}
