#pragma once

#include "media/FfmpegCliPopenMedia.hpp"
#include "tt/Sprite.hpp"
#include <SFML/Graphics.hpp>

namespace viz
{

/**
 * An `sf::Drawable` that displays the metadata of a song, typically provided by a `Media` struct.
 * By only keeping references to them, it allows you to customize the title and artist text as you see fit.
 * It will only update the texts' positions (to align with the album cover) and draw them.
 */
class SongMetadataDrawable : public sf::Drawable
{
	sf::Text &title_text, &artist_text;
	sf::Texture ac_txr;
	tt::Sprite ac_spr = ac_txr;

public:
	SongMetadataDrawable(sf::Text &title_text, sf::Text &artist_text);
	void use_metadata(const FfmpegCliMedia &);
	void set_album_cover(const sf::Texture &txr, const sf::Vector2f size);
	void set_position(const sf::Vector2f pos);
	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override;

private:
	void update_text_positions();
};

} // namespace viz
