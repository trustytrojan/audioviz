#pragma once

#include "Media.hpp"

class FfmpegCliMedia : public Media
{
public:
	// inherit constructor from base class!!!!!! nice feature!!!!!!!!!
	using Media::Media;

	void decode_audio(const int frames) override
	{
		const auto samples_to_read = frames * _astream.nb_channels();
		while (_audio_buffer.size() < samples_to_read)
		{
			float buf[samples_to_read];
			const auto samples_read = read_audio_samples(buf, samples_to_read);
			if (!samples_read)
				return;
			_audio_buffer.insert(_audio_buffer.end(), buf, buf + samples_read);
		}
	}
};
