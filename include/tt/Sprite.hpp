#pragma once

#include <SFML/Graphics.hpp>

namespace tt
{

struct Sprite : sf::Sprite
{
	Sprite(const sf::Texture &txr);
	void capture_centered_square_view();
	void scale_to(const sf::Vector2f size);
	void fill_screen(const sf::Vector2u size);
	sf::Vector2f get_size() const;
};

} // namespace tt
