#pragma once

extern "C"
{
#include <libavformat/avformat.h>
}

namespace av
{
	// Wrapper class over an `AVStream`.
	class Stream
	{
		const AVStream *_s;

	public:
		Stream(const AVStream *const _s) : _s(_s) {}

		/**
		 * @brief Access the internal `AVStream`.
		 * @return A read-only pointer to the internal `AVStream`.
		 */
		const AVStream *operator->() const { return _s; }

		/**
		 * Wrapper over `av_dict_get(_s->metadata, ...)`.
		 * @returns The value associated with `key` in stream `i`, or `NULL` if no entry exists for `key`.
		 */
		const char *metadata(const char *const key, const AVDictionaryEntry *prev = NULL, const int flags = 0) const
		{
			const auto entry = av_dict_get(_s->metadata, key, prev, flags);
			return entry ? entry->value : NULL;
		}

		/**
		 * @return The duration of the stream, in seconds.
		 */
		double duration_sec() const
		{
			return _s->duration * av_q2d(_s->time_base);
		}

		/**
		 * @return The total number of samples (per channel) in this stream.
		 */
		int samples() const
		{
			return duration_sec() * sample_rate();
		}

		/**
		 * @return The number of channels in this stream.
		 */
		int nb_channels() const
		{
			return _s->codecpar->ch_layout.nb_channels;
		}

		/**
		 * @return The sample rate of this stream.
		 */
		int sample_rate() const
		{
			return _s->codecpar->sample_rate;
		}
	};
}