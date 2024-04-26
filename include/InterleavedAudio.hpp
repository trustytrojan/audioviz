#pragma once

#include <vector>
#include <cstdio>
#include <stdexcept>

class InterleavedAudioBuffer
{
	std::vector<float> buf;
	const int n_channels;
	size_t pos = 0;

public:
	InterleavedAudioBuffer(const int n_channels)
		: n_channels(n_channels) {}
	
	std::vector<float> &buffer()
	{
		seek(0, SEEK_SET);
		return buf;
	}

	size_t read_frames(std::vector<float> &out, size_t n_frames)
	{
		const auto n_samples = std::min(n_frames * n_channels, buf.size() - pos);
		std::copy(buf.begin() + pos, buf.begin() + pos + n_samples, out.begin());
		if (n_samples < out.size())
			std::fill(out.begin() + n_samples, out.end(), 0.0f);
		pos += n_samples;
		return n_samples / n_channels;
	}

	void seek(size_t frame, int whence)
	{
		switch (whence)
		{
		case SEEK_SET:
			pos = std::min(frame * n_channels, buf.size());
			break;
		case SEEK_CUR:
			pos = std::min(pos + frame * n_channels, buf.size());
			break;
		case SEEK_END:
			pos = std::min(buf.size() - frame * n_channels, buf.size());
			break;
		default:
			throw std::invalid_argument("unknown whence value: " + std::to_string(whence));
		}
	}
};