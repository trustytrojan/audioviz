#pragma once

#include "Demuxer.hpp"
#include "Error.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
}

namespace av
{
	class StreamDecoder
	{
		friend class Demuxer;
		using FrameCallback = std::function<void(const AVFrame *)>;

		const AVStream *const stream;
		const AVCodecParameters *const codecpar = stream->codecpar;
		const AVCodec *const codec = avcodec_find_decoder(codecpar->codec_id);
		AVCodecContext *cdctx = avcodec_alloc_context3(codec);
		AVFrame *frame = av_frame_alloc();
		FrameCallback frame_callback;

		StreamDecoder(const AVStream *const stream)
			: stream(stream)
		{
			if (!codec)
				throw std::runtime_error("could not find decoder");
			if (!cdctx)
				throw std::runtime_error("could not allocate codec context");
			if (!frame)
				throw std::runtime_error("could not allocate frame");
		}

	public:
		~StreamDecoder()
		{
			av_frame_free(&frame);
			avcodec_free_context(&cdctx);
		}

		bool is_open() const
		{
			return avcodec_is_open(cdctx);
		}

		double duration_sec() const
		{
			return stream->duration * av_q2d(stream->time_base);
		}

		AVCodecID codec_id() const
		{
			return codec->id;
		}

		AVMediaType media_type() const
		{
			return codec->type;
		}

		/**
		 * Returns a pointer to the internal `AVCodecContext`, so that the caller may configure the decoder before calling `open`.
		 * ONLY MODIFY WHAT YOU NEED FOR CALLING `open`, AND NOTHING ELSE.
		 */
		AVCodecContext *get_cdctx()
		{
			return cdctx;
		}

		/**
		 * Opens the internal `AVCodecContext` for decoding.
		 * @param callback callback to receive each decoded `AVFrame *`
		 */
		void open(const FrameCallback callback)
		{
			if (const auto rc = avcodec_open2(cdctx, codec, NULL); rc < 0)
				throw Error("avcodec_open2", rc);
			frame_callback = callback;
		}

	private:
		/**
		 * Only to be called by `Demuxer`.
		 */
		bool decode(const AVPacket *const packet)
		{
			if (!is_open())
				// it is assumed that the caller doesn't want this stream's data.
				// return true to allow loops to continue reading from the owning `Demuxer`.
				return true;
			if (packet->stream_index != stream->index)
				// this should never happen unless `Demuxer` gave us the wrong `AVStream`
				throw std::runtime_error("received packet is not for this stream!");
			if (!cd_send_packet(packet))
				// decoder has been flushed; cannot continue decoding
				return false;
			while (cd_recv_frame())
				frame_callback(frame);
		}

		bool cd_send_packet(const AVPacket *const packet)
		{
			switch (const auto rc = avcodec_send_packet(cdctx, packet))
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
			switch (const auto rc = avcodec_receive_frame(cdctx, frame))
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
