#include "include/PortAudio.hpp"
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

public:
	AudioDecoder(const char *const url)
	{
		fmtctx = avformat_alloc_context();

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
		avcodec_free_context(&cdctx);
		avformat_close_input(&fmtctx);
	}

	double duration() const
	{
		return stream->duration * av_q2d(stream->time_base);
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
			timestamp = av_rescale_q(cdctx->frame_number + frame, {1, 1}, stream->time_base);
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

	int read_frames(float *const out, const int frames)
	{
		const auto out_size = frames * nb_channels();
		auto out_idx = 0;

		auto packet = av_packet_alloc();
		auto frame = av_frame_alloc();

		while (out_idx < out_size && fmt_read_frame(packet))
		{
			if (packet->stream_index != stream_idx)
				continue;

			if (!cd_send_packet(packet))
				break;

			while (out_idx < out_size && cd_recv_frame(frame))
			{
				const auto data = reinterpret_cast<const float *>(frame->extended_data[0]);
				const auto n_samples = std::min(frame->nb_samples * nb_channels(), out_size - out_idx);
				std::copy(data, data + n_samples, out + out_idx);
				out_idx += n_samples;
			}

			// av_packet_unref(packet);
		}

		av_frame_free(&frame);
		av_packet_free(&packet);

		return out_idx / nb_channels();
	}

private:
	bool fmt_read_frame(AVPacket *const packet)
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
	 * @returns true if successful or data needs to be read from the decoder; false if EOF
	 * @throws `AVError` on any other return code
	 */
	bool cd_send_packet(AVPacket *const packet)
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
	 * @returns true if successful; false if EOF or nothing to receive
	 * @throws `AVError` on any other return code
	 */
	bool cd_recv_frame(AVFrame *const frame)
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

int main(const int argc, const char *const *const argv)
{
	if (argc < 2)
	{
		std::cerr << "audio file required\n";
		return EXIT_FAILURE;
	}

	AudioDecoder decoder(argv[1]);
	size_t sample_size = decoder.duration() * decoder.sample_rate();

	PortAudio _;
	PortAudio::Stream pa_stream(0, decoder.nb_channels(), paFloat32, decoder.sample_rate(), sample_size);

	auto audio_buf = new float[decoder.nb_channels() * sample_size];

	if (decoder.read_frames(audio_buf, sample_size) != sample_size)
		throw std::runtime_error("didn't decode entire file!");

	pa_stream.write(audio_buf, sample_size);
}