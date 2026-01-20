#pragma once

#include "Base.hpp"
#include "media/Media.hpp"

namespace audioviz
{

class Player
{
	Base &viz;
	Media &media;
	const int framerate;
	const int audio_frames_needed; // usually needed for FFT

	// audio frames per video frame
	const int afpvf{media.audio_sample_rate() / framerate};

public:
	Player(Base &viz, Media &media, int framerate, int audio_frames_needed);

	void start_in_window(const std::string &title);
	void encode(const std::string &outfile, const std::string &vcodec, const std::string &acodec);
};

} // namespace audioviz
