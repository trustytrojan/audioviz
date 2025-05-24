#include <audioviz/Sprite.hpp>

namespace audioviz
{

Sprite::Sprite(const sf::Texture &txr)
	: sf::Sprite(txr)
{
}

void Sprite::capture_centered_square_view()
{
	const auto tsize = static_cast<sf::Vector2i>(getTexture().getSize());
	if (tsize.x == tsize.y)
		return;
	const auto square_size = std::min(tsize.x, tsize.y);
	setTextureRect({{tsize.x / 2.f - square_size / 2.f, 0}, {square_size, square_size}});
}

void Sprite::scale_to(const sf::Vector2f size)
{
	const auto trsize = getTextureRect().size;
	const auto scale_factor = std::min(size.x / trsize.x, size.y / trsize.y);
	setScale(sf::Vector2f{scale_factor, scale_factor});
}

void Sprite::fill_screen(const sf::Vector2u size)
{
	setOrigin((sf::Vector2f)getTextureRect().size / 2.f);
	setPosition((sf::Vector2f)size / 2.f);
	const float max_dim = std::max(size.x, size.y);
	scale_to({max_dim, max_dim});
}

sf::Vector2f Sprite::get_size() const
{
	return sf::Vector2f(getTextureRect().size).componentWiseMul(getScale());
}

} // namespace audioviz
