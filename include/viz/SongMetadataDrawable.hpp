#pragma once

#include "tt/Sprite.hpp"
#include <SFML/Graphics.hpp>

namespace viz
{

class SongMetadataDrawable : public sf::Drawable
{
public:
	sf::Text title_text, artist_text;

private:
	sf::Texture ac_txr;
	tt::Sprite ac_spr = ac_txr;

public:
	SongMetadataDrawable(const sf::Font &font);
	void set_album_cover(const sf::Texture &txr, const sf::Vector2f size);
	void set_position(const sf::Vector2f pos);
	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override;
};

} // namespace viz
