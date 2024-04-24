#include <stdexcept>
#include <deque>
#include <vector>
#include <iostream>
#include <cassert>

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

	std::vector<float> buf;

public:
	AudioDecoder(const char *const url)
	{
		if (const auto ret = avformat_open_input(&fmtctx, url, NULL, NULL); ret < 0)
			throw AVError("avformat_open_input", ret);

		if (const auto ret = avformat_find_stream_info(fmtctx, NULL); ret < 0)
			throw AVError("avformat_find_stream_info", ret);

		stream_idx = av_find_best_stream(fmtctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

		if (stream_idx < 0)
			throw AVError("av_find_best_stream", stream_idx);

		stream = fmtctx->streams[stream_idx];
		codecpar = stream->codecpar;

		auto codec = avcodec_find_decoder(codecpar->codec_id);

		if (!codec)
			throw std::runtime_error("could not find decoder");

		cdctx = avcodec_alloc_context3(codec);

		if (!cdctx)
			throw std::runtime_error("could not allocate codec context");

		cdctx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
		avcodec_open2(cdctx, codec, NULL);
	}

	AudioDecoder(const std::string &url)
		: AudioDecoder(url.c_str()) {}

	~AudioDecoder()
	{
		av_frame_free(&frame);
		av_packet_free(&packet);
		avcodec_free_context(&cdctx);
		avformat_close_input(&fmtctx);
	}

	constexpr double duration_sec() const
	{
		return stream->duration * ((double)stream->time_base.num / stream->time_base.den);
	}

	constexpr int frames() const
	{
		return duration_sec() * sample_rate();
	}

	constexpr int nb_channels() const
	{
		return codecpar->ch_layout.nb_channels;
	}

	constexpr int sample_rate() const
	{
		return codecpar->sample_rate;
	}

	void seek(int64_t frame, int whence) const
	{
		int64_t timestamp;

		switch (whence)
		{
		case SEEK_SET:
			timestamp = frame;
			break;
		case SEEK_CUR:
			timestamp = av_rescale_q(cdctx->frame_num + frame, {1, 1}, stream->time_base);
			break;
		case SEEK_END:
			timestamp = av_rescale_q(stream->duration - frame, {1, 1}, stream->time_base);
			break;
		default:
			throw std::invalid_argument("invalid seek mode");
		}

		if (const auto ret = av_seek_frame(fmtctx, stream_idx, timestamp, 0); ret < 0)
			throw AVError("av_seek_frame", ret);

		avcodec_flush_buffers(cdctx);
	}

	/**
	 * Decodes one frame from the audio format/container, and writes the decoded audio to `out`.
	 * The audio data will contain non-planar/packed/interleaved 32-bit floating point samples.
	 * It is unknown how many samples are written to `out`, as it depends on the codec.
	 * @param out vector to write audio data to. will be cleared before writing audio data.
	 */
	bool operator>>(std::vector<float> &out)
	{
		if (!fmt_read_frame() || !cd_send_packet())
			return false;
		out.clear();
		while (cd_recv_frame())
		{
			const auto data = (const float *)frame->extended_data[0];
			out.insert(out.end(), data, data + frame->linesize[0]);
		}
		return out.size();
	}

	bool read(std::vector<float> &out, size_t n_frames)
	{
		const auto n_samples = n_frames * nb_channels();

		if (!buf.empty())
		{
			out.insert(out.end(), buf.cbegin(), buf.cend());
		}

		while (out.size() < n_samples && *this >> buf)
		{
			out.insert(out.end(), buf.cbegin(), buf.cend());
		}
	}

private:
	/**
	 * @returns `true` on success; `false` on `AVERROR_EOF`
	 * @throws `AVError` on any other return code
	 */
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

	/**
	 * wrapper over `avcodec_send_packet`
	 * @returns `true` on success or `AVERROR(EAGAIN)`; `false` on `AVERROR_EOF`
	 * @throws `AVError` on any other return code
	 */
	bool cd_send_packet()
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

	/**
	 * wrapper over `avcodec_receive_frame`
	 * @returns `true` on success; `false` on `AVERROR_EOF` or `AVERROR(EAGAIN)`
	 * @throws `AVError` on any other return code
	 */
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