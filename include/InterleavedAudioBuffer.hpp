#pragma once

#include <vector>
#include <cstdio>
#include <stdexcept>
#include <ranges>

// now uses spans to reduce memory usage and unnecessary copying!
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
		return buf;
	}

	size_t frames_available()
	{
		return (buf.size() - pos) / n_channels;
	}

	size_t read_frames(std::span<float> &span, size_t n_frames)
	{
		const auto n_samples = std::min(n_frames * n_channels, buf.size() - pos);
		span = std::span(buf).subspan(pos, n_samples);
		pos += n_samples;
		return span.size() / 2;
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