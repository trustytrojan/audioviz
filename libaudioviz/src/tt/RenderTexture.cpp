#include "tt/RenderTexture.hpp"

namespace tt
{

RenderTexture::RenderTexture(const sf::Vector2u size, int antialiasing)
	: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}}
{
}

// Copy the contents of `other` to this render-texture.
void RenderTexture::copy(const RenderTexture &other)
{
	draw(other.sprite());
	display();
}

} // namespace tt
