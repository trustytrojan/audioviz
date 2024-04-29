#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include "Error.hpp"
#include "StreamDecoder.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace av
{
	class Demuxer
	{
		AVFormatContext *fmtctx;
		std::vector<StreamDecoder> decoders;
		AVPacket *packet = av_packet_alloc();

		Demuxer(const char *const url)
		{
			if (const auto rc = avformat_open_input(&fmtctx, url, NULL, NULL); rc < 0)
				throw Error("avformat_open_input", rc);

			if (const auto rc = avformat_find_stream_info(fmtctx, NULL); rc < 0)
				throw Error("avformat_find_stream_info", rc);

			for (int i = 0; i < fmtctx->nb_streams; ++i)
				decoders.emplace_back(decoders.begin() + i, fmtctx->streams[i]);
		}

		~Demuxer()
		{
			av_packet_free(&packet);
			avformat_close_input(&fmtctx);
		}

		StreamDecoder &find_best_stream(const AVMediaType type)
		{
			const auto rc = av_find_best_stream(fmtctx, type, -1, -1, NULL, 0);
			if (rc < 0)
				throw Error("av_find_best_stream", rc);
			return decoders.at(rc);
		}

		StreamDecoder &get_stream(int i)
		{
			return decoders.at(i);
		}

		bool read_frame_and_dispatch_packet()
		{
			if (!fmt_read_frame())
				return false;
			auto &decoder = decoders.at(packet->stream_index);
			if (!decoder.is_open())
				// it is assumed the caller does not want this stream decoded.
				// return `true` as to not confuse callers with hitting EOF.
				return true;
			return decoder.decode(packet);
		}

	private:
		bool fmt_read_frame()
		{
			switch (const auto rc = av_read_frame(fmtctx, packet))
			{
			case 0:
				return true;
			case AVERROR_EOF:
				return false;
			default:
				throw Error("av_read_frame", rc);
			}
		}
	};
}