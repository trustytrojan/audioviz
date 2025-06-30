#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/Base.hpp>

namespace audioviz
{

class FfmpegEncoder
{
public:
	virtual ~FfmpegEncoder() = default;
	virtual void send_frame(const sf::Image &img) = 0;
	inline virtual void send_frame(const sf::Texture &txr) { send_frame(txr.copyToImage()); }
};

} // namespace audioviz
