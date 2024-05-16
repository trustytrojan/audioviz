#pragma once

#include <SFML/Graphics.hpp>

namespace tt
{

struct Sprite : sf::Sprite
{
	Sprite(const sf::Texture &txr)
		: sf::Sprite(txr) {}

	void capture_centered_square_view()
	{
		const auto tsize = static_cast<sf::Vector2i>(getTexture().getSize());
		if (tsize.x == tsize.y)
			return;
		const auto square_size = std::min(tsize.x, tsize.y);
		setTextureRect({{tsize.x / 2.f - square_size / 2.f, 0}, {square_size, square_size}});
	}

	void set_size(const sf::Vector2f desired)
	{
		const auto tsize = getTextureRect().getSize();
		const auto scale_factor = std::min(desired.x / tsize.x, desired.y / tsize.y);
		setScale({scale_factor, scale_factor});
	}

	void fill_screen(const sf::Vector2u size)
	{
		setOrigin((sf::Vector2f)getTextureRect().getSize() / 2.f);
		setPosition((sf::Vector2f)size / 2.f);
		const float max_dim = std::max(size.x, size.y);
		set_size({max_dim, max_dim});
	}
};

} // namespace tt
