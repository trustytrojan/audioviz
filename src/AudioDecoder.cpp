#include "AudioDecoder.hpp"

AudioDecoder::AudioDecoder(const char *const url)
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

AudioDecoder::AudioDecoder(const std::string &url)
	: AudioDecoder(url.c_str()) {}

AudioDecoder::~AudioDecoder()
{
	av_frame_free(&frame);
	av_packet_free(&packet);
	avcodec_free_context(&cdctx);
	avformat_close_input(&fmtctx);
}

double AudioDecoder::duration_sec() const
{
	return stream->duration * av_q2d(stream->time_base);
}

int AudioDecoder::frames() const
{
	return duration_sec() * sample_rate();
}

int AudioDecoder::nb_channels() const
{
	return codecpar->ch_layout.nb_channels;
}

int AudioDecoder::sample_rate() const
{
	return codecpar->sample_rate;
}

const char *AudioDecoder::get_metadata_entry(const char *const key, const AVDictionaryEntry *const prev, const int flags) const
{
	return av_dict_get(fmtctx->metadata, key, NULL, 0)->value;
}

void AudioDecoder::seek(int64_t frame, int whence)
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

bool AudioDecoder::operator>>(std::vector<float> &out)
{
	if (!fmt_read_frame() || !cd_send_packet())
		return false;
	out.clear();
	while (cd_recv_frame())
	{
		const auto data = (const float *)frame->extended_data[0];
		const auto nb_floats = frame->nb_samples * nb_channels();
		out.insert(out.end(), data, data + nb_floats);
	}
	return out.size();
}

bool AudioDecoder::fmt_read_frame()
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

bool AudioDecoder::cd_send_packet()
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

bool AudioDecoder::cd_recv_frame()
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