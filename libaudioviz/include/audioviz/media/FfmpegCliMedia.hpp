#pragma once

#include <audioviz/media/Media.hpp>

namespace audioviz::media
{

class FfmpegCliMedia : public Media
{
public:
	// inherit constructor from base class!!!!!! nice feature!!!!!!!!!
	using Media::Media;

	void decode_audio(const int frames) override;
};

} // namespace audioviz::media
