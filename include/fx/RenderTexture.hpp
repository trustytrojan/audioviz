#pragma once

#include <SFML/Graphics.hpp>
#include "Effect.hpp"
#include "../tt/RenderTexture.hpp"

namespace fx
{
	// An `sf::RenderTexture` that allows for easy post-processing effects.
	struct RenderTexture : public tt::RenderTexture
	{
		RenderTexture(const sf::Vector2u size, int antialiasing = 0)
			: tt::RenderTexture(size, antialiasing) {}

		void apply(const Effect &effect)
		{
			effect.apply(*this);
		}
	};
}
