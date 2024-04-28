#pragma once

#include <vector>
#include <mutex>
#include "AVError.hpp"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

// class to decode audio from a url into non-planar f32 frames
class AudioDecoder
{
	AVFormatContext *fmtctx = nullptr;
	int stream_idx = -1;
	const AVStream *stream = nullptr;			 // freed by `avformat_close_input`
	const AVCodecParameters *codecpar = nullptr; // freed by `avformat_close_input`
	AVCodecContext *cdctx = nullptr;
	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();
	std::vector<float> buf;
	int current_frame = 0;

public:
	AudioDecoder(const char *const url);
	AudioDecoder(const std::string &url);
	~AudioDecoder();

	double duration_sec() const;
	int frames() const;
	int nb_channels() const;
	int sample_rate() const;

	/**
	 * wrapper over `av_dict_get` with `fmtctx->metadata`
	 * @returns value associated with `key`
	 */
	const char *get_metadata_entry(const char *const key, const AVDictionaryEntry *prev = NULL, const int flags = 0) const;

	/**
	 * WARNING: this advances the position in the `AVFormatContext`.
	 * decodes one frame from the format and appends the samples to `out`.
	 * @param out vector to append decoded samples to
	 * @param mtx optional mutex to lock when appending to `out`.
	 * @return whether anything was decoded into `out`; `false` most likely means we have reached end-of-file
	 */
	bool append_to(std::vector<float> &out, std::mutex *mtx = NULL);

private:
	/**
	 * wrapper over `av_read_frame`
	 * @returns `true` on success; `false` on `AVERROR_EOF`
	 * @throws `AVError` on any other return code
	 */
	bool fmt_read_frame();

	/**
	 * wrapper over `avcodec_send_packet`
	 * @returns `true` on success or `AVERROR(EAGAIN)`; `false` on `AVERROR_EOF`
	 * @throws `AVError` on any other return code
	 */
	bool cd_send_packet();

	/**
	 * wrapper over `avcodec_receive_frame`
	 * @returns `true` on success; `false` on `AVERROR_EOF` or `AVERROR(EAGAIN)`
	 * @throws `AVError` on any other return code
	 */
	bool cd_recv_frame();
};