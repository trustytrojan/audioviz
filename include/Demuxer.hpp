#pragma once

#include <vector>
#include <mutex>
#include <functional>
#include "AVError.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class StreamDecoder
{
	using FrameCallback = std::function<void(const AVFrame *)>;

	const AVStream *const stream;
	const AVCodecParameters *const codecpar = stream->codecpar;
	const AVCodec *const codec = avcodec_find_decoder(codecpar->codec_id);
	AVCodecContext *cdctx = avcodec_alloc_context3(codec);
	AVFrame *frame = av_frame_alloc();
	FrameCallback frame_callback;

public:
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

	~StreamDecoder()
	{
		av_frame_free(&frame);
		avcodec_free_context(&cdctx);
	}

	bool cdctx_is_open() const
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

	void set_frame_callback(const FrameCallback callback)
	{
		frame_callback = callback;
	}

	// Returns the internal `AVCodecContext *`.
	// ONLY MODIFY WHAT YOU NEED FOR CALLING `open_codec`, AND NOTHING ELSE.
	AVCodecContext *get_cdctx()
	{
		return cdctx;
	}

	void open_codec()
	{
		if (const auto rc = avcodec_open2(cdctx, codec, NULL); rc < 0)
			throw AVError("avcodec_open2", rc);
	}

	// `open_codec` must be called before calling `decode`.
	// Otherwise `std::runtime_error` will be thrown.
	// Also, you must call `set_frame_callback` to receive decoded data.
	bool decode(const AVPacket *const packet)
	{
		if (!cdctx_is_open())
			throw std::runtime_error("codec context is not open!");
		if (packet->stream_index != stream->index)
			throw std::runtime_error("packet is not for this stream!");
		if (!cd_send_packet(packet))
			return false;
		while (cd_recv_frame())
			if (frame_callback)
				frame_callback(frame);
	}

private:
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
			throw AVError("avcodec_send_packet", rc);
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
			throw AVError("avcodec_receive_frame", rc);
		}
	}
};

class MediaFileReader
{
	AVFormatContext *fmtctx;
	std::vector<StreamDecoder> decoders;
	AVPacket *packet = av_packet_alloc();

	MediaFileReader(const char *const url)
	{
		if (const auto rc = avformat_open_input(&fmtctx, url, NULL, NULL); rc < 0)
			throw AVError("avformat_open_input", rc);

		if (const auto rc = avformat_find_stream_info(fmtctx, NULL); rc < 0)
			throw AVError("avformat_find_stream_info", rc);

		for (int i = 0; i < fmtctx->nb_streams; ++i)
			decoders.emplace(decoders.begin() + i, fmtctx->streams[i]);
	}

	~MediaFileReader()
	{
		av_packet_free(&packet);
		avformat_close_input(&fmtctx);
	}

	StreamDecoder &find_best_stream(const AVMediaType type)
	{
		const auto rc = av_find_best_stream(fmtctx, type, -1, -1, NULL, 0);
		if (rc < 0)
			throw AVError("av_find_best_stream", rc);
		return decoders.at(rc);
	}

	StreamDecoder &stream(int i)
	{
		return decoders.at(i);
	}

	bool read_frame_and_dispatch_packet()
	{
		if (!fmt_read_frame())
			return false;
		auto &decoder = decoders.at(packet->stream_index);
		// if the caller didn't open a decoder, just skip it
		if (!decoder.cdctx_is_open())
			return true;
		return decoder.decode(packet);;
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
			throw AVError("av_read_frame", rc);
		}
	}
};