#pragma once

#include <SFML/Graphics.hpp>
#include <avz/Sprite.hpp>
#include <avz/media/Media.hpp>

namespace avz
{

/**
 * An `sf::Drawable` that displays the metadata of a song, typically provided by a `Media` struct.
 * By only keeping references to them, it allows you to customize the title and artist text as you see fit.
 * It will only update the texts' positions (to align with the album cover) and draw them.
 */
class SongMetadataDrawable : public sf::Drawable
{
public:
	enum class TextPosition
	{
		RIGHT,
		BOTTOM
	};

private:
	sf::Text &title_text, &artist_text;
	sf::Texture ac_txr;
	Sprite ac_spr = ac_txr;
	TextPosition text_pos{TextPosition::RIGHT};

public:
	SongMetadataDrawable(sf::Text &title_text, sf::Text &artist_text);
	void use_metadata(const Media &);
	void set_album_cover(const sf::Texture &txr, const sf::Vector2f size);
	void set_position(const sf::Vector2f pos);
	inline sf::Vector2f get_ac_spr_center() { return ac_spr.get_center(); }
	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override;

	inline void set_text_pos(const TextPosition tp)
	{
		text_pos = tp;
		update_text_positions();
	}

private:
	void update_text_positions();
};

} // namespace avz
