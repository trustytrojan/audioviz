#include "viz/SongMetadataDrawable.hpp"

namespace viz
{

SongMetadataDrawable::SongMetadataDrawable(const sf::Font &font)
	: title_text(font),
	  artist_text(font)
{
}

void SongMetadataDrawable::use_metadata(const Media &media)
{
	if (const auto title = media._format.metadata("title"))
		title_text.setString(title);
	else if (const auto title = media._astream.metadata("title"))
		title_text.setString(title);

	if (const auto artist = media._format.metadata("artist"))
		artist_text.setString(artist);
	else if (const auto artist = media._astream.metadata("artist"))
		artist_text.setString(artist);
}

void SongMetadataDrawable::set_album_cover(const sf::Texture &txr, const sf::Vector2f size)
{
	ac_txr = txr;
	ac_spr.capture_centered_square_view();
	ac_spr.scale_to(size);
}

void SongMetadataDrawable::set_position(const sf::Vector2f pos)
{
	ac_spr.setPosition(pos);
	const auto ac_size = ac_spr.get_size();
	const sf::Vector2f text_pos{pos.x + ac_size.x + 10 * bool(ac_size.x), pos.y};
	title_text.setPosition({text_pos.x, text_pos.y});
	artist_text.setPosition({text_pos.x, text_pos.y + title_text.getCharacterSize() + 5});
}

void SongMetadataDrawable::draw(sf::RenderTarget &target, const sf::RenderStates states) const
{
	if (ac_txr.getNativeHandle())
		target.draw(ac_spr, states);
	if (!title_text.getString().isEmpty())
		target.draw(title_text, states);
	if (!artist_text.getString().isEmpty())
		target.draw(artist_text, states);
}

} // namespace viz
