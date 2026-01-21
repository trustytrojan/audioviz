#pragma once

#include <avz/gfx/util.hpp>
#include <SFML/Graphics.hpp>
#include <avz/gfx/Sprite.hpp>

namespace avz
{

/**
 * An `sf::Drawable` that displays the metadata of a song, in a specific style.
 * It takes references to the `sf::Text` objects for the artist & title.
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
	inline void set_title(const std::string &title) { title_text.setString(util::utf8_to_sf_string(title)); }
	inline void set_artist(const std::string &artist) { artist_text.setString(util::utf8_to_sf_string(artist)); }
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
