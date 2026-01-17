#include <audioviz/RenderTexture.hpp>

namespace audioviz
{

RenderTexture::RenderTexture(const sf::Vector2u size, unsigned antialiasing)
	: sf::RenderTexture{size, {.antiAliasingLevel = antialiasing}}
{
}

void RenderTexture::copy(const RenderTexture &other)
{
	draw(other.sprite());
	display();
}

} // namespace audioviz
