#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include <span>

#include "Error.hpp"
#include "Stream.hpp"

extern "C"
{
#include <libavformat/avformat.h>
}

namespace av
{
	// Wrapper class over an `AVFormatContext`, specialized for reading media files.
	class MediaFileReader
	{
		AVFormatContext *_fmtctx = NULL;
		AVPacket *_pkt = NULL;
		std::vector<Stream> _streams;

	public:
		MediaFileReader(const char *const url)
		{
			if (const auto rc = avformat_open_input(&_fmtctx, url, NULL, NULL); rc < 0)
				throw Error("avformat_open_input", rc);
			if (const auto rc = avformat_find_stream_info(_fmtctx, NULL); rc < 0)
				throw Error("avformat_find_stream_info", rc);
			_streams.assign(_fmtctx->streams, _fmtctx->streams + _fmtctx->nb_streams);
		}

		MediaFileReader(const std::string &url)
			: MediaFileReader(url.c_str()) {}

		~MediaFileReader()
		{
			av_packet_free(&_pkt);
			avformat_close_input(&_fmtctx);
		}

		/**
		 * @brief Access fields of the internal `AVFormatContext`.
		 * @return A read-only pointer to the internal `AVFormatContext`.
		 */
		const AVFormatContext *operator->() const { return _fmtctx; }

		/**
		 * Wrapper over `av_dict_get(fmtctx->metadata, ...)`.
		 * @returns The string value associated with `key`, or `NULL` if no entry exists for `key`.
		 */
		const char *metadata(const char *const key, const AVDictionaryEntry *prev = NULL, const int flags = 0) const
		{
			const auto entry = av_dict_get(_fmtctx->metadata, key, prev, flags);
			return entry ? entry->value : NULL;
		}

		const std::vector<Stream> &streams() const
		{
			return _streams;
		}

		// Wrapper over `av_find_best_stream(fmtctx, ...)`.
		/**
		 * Wrapper over `av_find_best_stream(fmtctx, ...)`.
		 * @returns The highest quality stream of media type `type`.
		 * @throws `av::Error` if `av_find_best_stream` fails
		 */
		Stream find_best_stream(const AVMediaType type, int wanted_stream_nb = -1, int related_stream = -1, const AVCodec **decoder_ret = NULL, int flags = 0) const
		{
			const auto idx = av_find_best_stream(_fmtctx, type, wanted_stream_nb, related_stream, decoder_ret, flags);
			if (idx < 0)
				throw Error("av_find_best_stream", idx);
			return _fmtctx->streams[idx];
		}

		/**
		 * @brief Read a packet of data from the media file.
		 * 
		 * @return A pointer to the internal `AVPacket`, or `NULL` if end-of-file has been reached.
		 * **Do not free the returned pointer.**
		 * 
		 * @note Each packet read from the file belongs to one of the streams in this media file.
		 * If you are decoding a stream, make sure to check the `stream_index` field of a packet
		 * to make sure you are giving the right packet to the right decoder.
		 */
		const AVPacket *read_packet()
		{
			if (!_pkt && !(_pkt = av_packet_alloc()))
				throw std::runtime_error("av_packet_alloc() failed");
			switch (const auto rc = av_read_frame(_fmtctx, _pkt))
			{
			case 0:
				return _pkt;
			case AVERROR_EOF:
				return NULL;
			default:
				throw Error("av_read_frame", rc);
			}
		}
	};
}