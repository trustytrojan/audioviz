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

	inline sf::Vector2f get_size() const { return sf::Vector2f{getTextureRect().size}.componentWiseMul(getScale()); }
	inline sf::Vector2f get_center() const { return getPosition() + (get_size() / 2.f); }
};

} // namespace audioviz
