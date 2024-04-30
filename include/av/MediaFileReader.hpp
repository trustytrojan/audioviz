#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include "Error.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace av
{
	class MediaFileReader
	{
		AVFormatContext *fmtctx = nullptr;
		AVPacket *packet = nullptr;
		std::vector<const AVStream *> _streams;

	public:
		MediaFileReader(const char *const url)
		{
			if (const auto rc = avformat_open_input(&fmtctx, url, NULL, NULL); rc < 0)
				throw Error("avformat_open_input", rc);

			if (const auto rc = avformat_find_stream_info(fmtctx, NULL); rc < 0)
				throw Error("avformat_find_stream_info", rc);

			_streams.assign(fmtctx->streams, fmtctx->streams + fmtctx->nb_streams);
		}

		MediaFileReader(const std::string &url)
			: MediaFileReader(url.c_str()) {}

		~MediaFileReader()
		{
			av_packet_free(&packet);
			avformat_close_input(&fmtctx);
		}

		const std::vector<const AVStream *> &streams() const
		{
			return _streams;
		}

		const AVStream *find_best_stream(const AVMediaType type, const AVCodec **const decoder_ret = NULL) const
		{
			const auto idx = av_find_best_stream(fmtctx, type, -1, -1, decoder_ret, 0);
			if (idx < 0)
				throw Error("av_find_best_stream", idx);
			return fmtctx->streams[idx];
		}

		/**
		 * @brief Read a packet of data from the media source.
		 * @return a pointer to the internal `AVPacket`, or `NULL` if end-of-file has been reached
		 */
		const AVPacket *read_packet()
		{
			if (!packet && !(packet = av_packet_alloc()))
				throw std::runtime_error("av_packet_alloc() failed");
			switch (const auto rc = av_read_frame(fmtctx, packet))
			{
			case 0:
				return packet;
			case AVERROR_EOF:
				return NULL;
			default:
				throw Error("av_read_frame", rc);
			}
		}
	};
}