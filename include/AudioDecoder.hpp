#pragma once

#include <deque>
#include <vector>
#include <stdexcept>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class AVError : public std::runtime_error
{
	static std::string get_err_string(const int errnum)
	{
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, errnum);
		return errbuf;
	}

public:
	const char *const func;
	const int errnum;

	AVError(const char *const func, const int errnum)
		: std::runtime_error(std::string(func) + ": " + get_err_string(errnum)),
		  func(func),
		  errnum(errnum) {}
};

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
	std::deque<float> buf;
	int current_frame = 0;

public:
	AudioDecoder(const char *const url);
	AudioDecoder(const std::string &url);
	~AudioDecoder();

	double duration_sec() const;
	int frames() const;
	int nb_channels() const;
	int sample_rate() const;

	const char *get_metadata_entry(const char *const key, const AVDictionaryEntry *prev = NULL, const int flags = 0) const;

	void decode_entire_file(std::vector<float> &out);
	int read_n_frames(std::vector<float> &out, int n_frames);

private:
	/**
	 * @return whether anything was decoded into `buf`
	 */
	bool decode_to_buffer();

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