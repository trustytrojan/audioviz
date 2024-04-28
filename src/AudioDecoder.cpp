#include "AudioDecoder.hpp"

// public:

#include <iostream>

AudioDecoder::AudioDecoder(const char *const url)
{
	if (const auto ret = avformat_open_input(&fmtctx, url, NULL, NULL); ret < 0)
		throw AVError("avformat_open_input", ret);

	if (const auto ret = avformat_find_stream_info(fmtctx, NULL); ret < 0)
		throw AVError("avformat_find_stream_info", ret);

	const AVCodec *codec;
	stream_idx = av_find_best_stream(fmtctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);

	if (stream_idx < 0)
		throw AVError("av_find_best_stream", stream_idx);

	stream = fmtctx->streams[stream_idx];
	codecpar = stream->codecpar;
	cdctx = avcodec_alloc_context3(codec);

	if (!cdctx)
		throw std::runtime_error("could not allocate codec context");

	cdctx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
	avcodec_open2(cdctx, codec, NULL);

	if (cdctx->sample_fmt != cdctx->request_sample_fmt)
	{
		if (const auto rc =
				swr_alloc_set_opts2(
					&swr,
					&cdctx->ch_layout, // input
					AV_SAMPLE_FMT_FLT,
					cdctx->sample_rate,
					&cdctx->ch_layout, // output
					cdctx->sample_fmt,
					cdctx->sample_rate, 0, NULL);
			rc < 0)
			throw AVError("swr_alloc_set_opts2", rc);
		if (const auto rc = swr_init(swr); rc < 0)
			throw AVError("swr_init", rc);
	}
}

AudioDecoder::AudioDecoder(const std::string &url)
	: AudioDecoder(url.c_str()) {}

AudioDecoder::~AudioDecoder()
{
	if (swr) swr_free(&swr);
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
	// the desired metadata can either be in the stream or the format itself
	auto entry = av_dict_get(stream->metadata, key, prev, flags);
	if (!entry)
		entry = av_dict_get(fmtctx->metadata, key, prev, flags);
	return entry ? entry->value : NULL;
}

bool AudioDecoder::append_to(std::vector<float> &out, std::mutex *const mtx)
{
	// make sure we skip over streams that are not ours!
	bool frame_read;
	while ((frame_read = fmt_read_frame()) && packet->stream_index != stream_idx)
		;
	if (!frame_read || !cd_send_packet())
		return false;
	while (cd_recv_frame())
	{
		const float *data = nullptr;

		// if swr is not NULL, need to set data to new buffer and convert
		if (swr)
		{
			const auto bufsize = av_samples_get_buffer_size(
				NULL, frame->ch_layout.nb_channels, frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
			if (bufsize < 0)
				throw AVError("av_samples_get_buffer_size", bufsize);
			data = (const float *)av_malloc(bufsize);
			if (!data)
				throw std::runtime_error("cannot allocate output data buffer!");
			if (const auto rc = swr_convert(swr, (uint8_t **)&data, frame->nb_samples, (const uint8_t **)frame->extended_data, frame->nb_samples); rc < 0)
				throw AVError("swr_convert", rc);
		}
		// otherwise we can use the frame's data
		else
			data = (const float *)frame->extended_data[0];

		const auto nb_floats = frame->nb_samples * nb_channels();
		if (mtx)
			mtx->lock();
		out.insert(out.end(), data, data + nb_floats);
		if (mtx)
			mtx->unlock();
		
		if (swr && data)
			av_freep(&data);
	}
	return out.size();
}

// private:

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