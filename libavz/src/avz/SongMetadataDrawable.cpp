#include <avz/SongMetadataDrawable.hpp>
#include <avz/util.hpp>

namespace avz
{

SongMetadataDrawable::SongMetadataDrawable(sf::Text &title_text, sf::Text &artist_text)
	: title_text(title_text),
	  artist_text(artist_text)
{
}

void SongMetadataDrawable::use_metadata(const Media &media)
{
	if (const auto title = media.title(); !title.empty())
		title_text.setString(util::utf8_to_sf_string(title));
	if (const auto artist = media.artist(); !artist.empty())
		artist_text.setString(util::utf8_to_sf_string(artist));
}

void SongMetadataDrawable::set_album_cover(const sf::Texture &txr, const sf::Vector2f size)
{
	ac_txr = txr;
	ac_spr.capture_centered_square_view();
	ac_spr.scale_to(size);
	update_text_positions();
}

void SongMetadataDrawable::set_position(const sf::Vector2f pos)
{
	ac_spr.setPosition(pos);
	update_text_positions();
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

void SongMetadataDrawable::update_text_positions()
{
	const auto ac_pos = ac_spr.getPosition();
	const auto ac_size = ac_spr.get_size();
	switch (text_pos)
	{
	case TextPosition::RIGHT:
	{
		const sf::Vector2f pos{ac_pos.x + ac_size.x + 10 * bool(ac_size.x), ac_pos.y};
		title_text.setPosition({pos.x, pos.y});
		artist_text.setPosition({pos.x, pos.y + title_text.getCharacterSize() + 5});
		break;
	}
	case TextPosition::BOTTOM:
	{
		const sf::Vector2f pos{ac_pos.x, ac_pos.y + ac_size.y + 10 * bool(ac_size.x)};
		title_text.setPosition({pos.x, pos.y});
		artist_text.setPosition({pos.x, pos.y + title_text.getCharacterSize() + 5});
		break;
	}
	}
}

} // namespace avz
