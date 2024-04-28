#pragma once

#include "AVError.hpp"
#include <vector>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

// class to encode:
// - rgba pixels
// - non-planar/interleaved 32-bit floating point samples
class VideoEncoder
{
	AVFormatContext *fmtctx = nullptr;

	struct _av
	{
		AVCodecContext *cdctx = nullptr;
		AVStream *stream = nullptr;
		AVFrame *frame = av_frame_alloc();
		AVPacket *packet = av_packet_alloc();

		~_av()
		{
			av_frame_free(&frame);
			av_packet_free(&packet);
		}

		bool cd_send_frame()
		{
			switch (const auto rc = avcodec_send_frame(cdctx, frame))
			{
			case 0:
			case AVERROR(EAGAIN):
				return true;
			case AVERROR_EOF:
				return false;
			default:
				throw AVError("avcodec_send_frame", rc);
			}
		}

		const AVPacket *cd_recv_packet()
		{
			switch (const auto rc = avcodec_receive_packet(cdctx, packet))
			{
			case 0:
			case AVERROR(EAGAIN):
				return packet;
			case AVERROR_EOF:
				return NULL;
			default:
				throw AVError("avcodec_receive_packet", rc);
			}
		}
	} a, v;

public:
	int audio_frame_size() const
	{
		return a.cdctx->frame_size;
	}

	bool encode_audio(const float *const buf)
	{
		a.frame->extended_data[0] = (uint8_t *)buf;
		if (!a.cd_send_frame())
			return false;
		while (const auto packet = a.cd_recv_packet())
		{
			
		}
	}

	bool encode_video(const uint8_t *const buf)
	{
		v.frame->data[0] = (uint8_t *)buf;
		if (!v.cd_send_frame())
			return false;
		while (const auto packet = v.cd_recv_packet())
		{
			
		}
	}

private:
	void setup_frames()
	{
		a.frame->format = AV_SAMPLE_FMT_FLT;
		a.frame->nb_samples = a.cdctx->frame_size;
		a.frame->ch_layout.nb_channels = 2;
		// if (const auto ret = av_frame_get_buffer(a.frame, 0); ret < 0)
		// 	throw AVError("av_frame_get_buffer", ret);
		
		v.frame->format = AV_PIX_FMT_RGBA;
		v.frame->width = 1280;
		v.frame->height = 720;
	}

	void fmt_write_frame(AVPacket *const packet)
	{
		// switch (conat auto rc = av_write_frame)
	}
};
