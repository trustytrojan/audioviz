#pragma once

#include <SFML/Graphics.hpp>
#include "Effect.hpp"
#include "../MyRenderTexture.hpp"

namespace fx
{
	// An `sf::RenderTexture` that allows for easy post-processing effects.
	struct RenderTexture : public MyRenderTexture
	{
		RenderTexture(const sf::Vector2u size, int antialiasing = 0)
			: MyRenderTexture(size, antialiasing) {}

		void apply(const Effect &effect)
		{
			effect.apply(*this);
		}
	};
}
