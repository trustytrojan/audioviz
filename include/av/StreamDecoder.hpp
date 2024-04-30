#pragma once

#include <functional>
#include "Error.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace av
{
	class StreamDecoder
	{
		using FrameCallback = std::function<void(const AVFrame *)>;

	public:
		const AVStream *const stream;
		const AVCodecParameters *const cdpar = stream->codecpar;
		const AVCodec *const codec = avcodec_find_decoder(cdpar->codec_id);

	private:
		AVCodecContext *_cdctx = avcodec_alloc_context3(codec);
		AVFrame *frame = av_frame_alloc();

	public:
		StreamDecoder(const AVStream *const stream)
			: stream(stream)
		{
			if (!codec)
				throw std::runtime_error("could not find decoder");
			if (!_cdctx)
				throw std::runtime_error("could not allocate codec context");
			if (!frame)
				throw std::runtime_error("could not allocate frame");
		}

		~StreamDecoder()
		{
			av_frame_free(&frame);
			avcodec_free_context(&_cdctx);
		}

		/**
		 * Returns a pointer to the internal `AVCodecContext`, so that the caller may configure the decoder before calling `open`.
		 * **Only modify what you need for calling `open`.**
		 */
		AVCodecContext *cdctx()
		{
			return _cdctx;
		}

		const AVCodecContext *cdctx() const
		{
			return _cdctx;
		}

		/**
		 * Opens the internal `AVCodecContext` for decoding.
		 * @param callback receives a const-pointer each decoded `AVFrame`
		 * @throws `av::Error` if `avcodec_open2` fails
		 */
		void open()
		{
			if (const auto rc = avcodec_open2(_cdctx, codec, NULL); rc < 0)
				throw Error("avcodec_open2", rc);
		}

		/**
		 * @brief Sends a packet to the decoder.
		 * @param packet packet to send to decoder
		 * @return whether the send was successful
		 * @note `true` is also returned in the case that the decoder has
		 * output frames to return to the caller.
		 * @throws `std::runtime_error` if either the codec context is not open,
		 * or the packet's stream index does not match the decoder's stream index
		 * @throws `av::Error` if any `avcodec_*` functions fail
		 */
		bool send_packet(const AVPacket *const pkt)
		{
			if (!avcodec_is_open(_cdctx))
				throw std::runtime_error("codec context is not open!");
			if (pkt->stream_index != stream->index)
				throw std::runtime_error("received packet is not for this stream!");
			return cd_send_packet(pkt);
		}

		// Alias for `send_packet`
		bool operator<<(const AVPacket *const pkt)
		{
			return send_packet(pkt);
		}

		/**
		 * @brief Receive a frame from the decoder.
		 * This method should be called in a loop since one packet
		 * can be decoded into several frames.
		 * @return a pointer to the internal `AVFrame`,
		 * or `NULL` if the codec requires a new packet or has been fully flushed
		 * @throws `std::runtime_error` if the codec context is not open
		 * @throws `av::Error` if any `avcodec_*` functions fail
		 */
		const AVFrame *receive_frame()
		{
			if (!avcodec_is_open(_cdctx))
				throw std::runtime_error("codec context is not open!");
			return cd_recv_frame() ? frame : NULL;
		}

	private:
		bool cd_send_packet(const AVPacket *const pkt)
		{
			switch (const auto rc = avcodec_send_packet(_cdctx, pkt))
			{
			case 0:
			case AVERROR(EAGAIN):
				return true;
			case AVERROR_EOF:
				return false;
			default:
				throw Error("avcodec_send_packet", rc);
			}
		}

		bool cd_recv_frame()
		{
			switch (const auto rc = avcodec_receive_frame(_cdctx, frame))
			{
			case 0:
				return true;
			case AVERROR(EAGAIN):
			case AVERROR_EOF:
				return false;
			default:
				throw Error("avcodec_receive_frame", rc);
			}
		}
	};
}
