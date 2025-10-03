#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz
{

class Sprite : public sf::Sprite
{
public:
	Sprite(const sf::Texture &txr);
	// for the sole purpose of widescreen youtube thumbnails containing square album covers
	void capture_centered_square_view();
	void scale_to(const sf::Vector2f size);
	void fill_screen(const sf::Vector2u size);
	sf::Vector2f get_size() const;
};

} // namespace audioviz
