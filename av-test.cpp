#include <iostream>
#include <optional>
#include <cassert>

#include "include/av/MediaFileReader.hpp"
#include "include/av/StreamDecoder.hpp"
#include "include/av/Resampler.hpp"
#include "include/av/Vector.hpp"

#include "include/pa/PortAudio.hpp"
#include "include/pa/Stream.hpp"

void with_resampling(const char *const url)
{
	av::MediaFileReader format(url);
	const auto stream = format.find_best_stream(AVMEDIA_TYPE_AUDIO);
	av::StreamDecoder decoder(stream);

	std::optional<av::Vector<uint8_t>> rs_output_buffer;
	std::optional<av::Resampler> rs;

	const auto cdctx = decoder.cdctx();
	cdctx->request_sample_fmt = AV_SAMPLE_FMT_U8;
	decoder.open();

	if (cdctx->sample_fmt != cdctx->request_sample_fmt)
	{
		rs_output_buffer.emplace();													 // creates empty vector
		rs.emplace(&cdctx->ch_layout, cdctx->request_sample_fmt, cdctx->sample_rate, // output params
				   &cdctx->ch_layout, cdctx->sample_fmt, cdctx->sample_rate);		 // input params
	}

	pa::PortAudio _;
	pa::Stream pa_stream(0, decoder.cdpar->ch_layout.nb_channels,
						 paUInt8,
						 decoder.cdpar->sample_rate,
						 paFramesPerBufferUnspecified);

	std::cout << "requested sample format: " << av_get_sample_fmt_name(cdctx->request_sample_fmt)
			  << "\ncodec/sample_fmt: " << avcodec_get_name(decoder.codec->id) << '/' << av_get_sample_fmt_name(cdctx->sample_fmt)
			  << "\nresampling to: " << av_get_sample_fmt_name(cdctx->request_sample_fmt) << '\n';

	while (const auto packet = format.read_packet())
	{
		if (packet->stream_index != decoder.stream->index)
			continue;
		if (!decoder.send_packet(packet))
			break;
		while (const auto frame = decoder.receive_frame())
		{
			assert(cdctx->sample_fmt == frame->format);
			if (!rs_output_buffer || !rs)
				return;
			rs_output_buffer->resize(frame->nb_samples * frame->ch_layout.nb_channels);
			auto buf = rs_output_buffer->data();
			rs->convert((uint8_t **)&buf, frame->nb_samples,						// output
						(const uint8_t **)frame->extended_data, frame->nb_samples); // input
			pa_stream.write(buf, frame->nb_samples);
		}
	}
}

bool is_interleaved(const AVSampleFormat sample_fmt)
{
	if (sample_fmt >= 0 && sample_fmt <= 4)
		return true;
	if (sample_fmt >= 5 && sample_fmt <= 11)
		return false;
	throw std::runtime_error("is_interleaved: invalid sample format!");
}

PaSampleFormat avsf2pasf(const AVSampleFormat av)
{
	PaSampleFormat pa;

	switch (av)
	{
	case AV_SAMPLE_FMT_U8:
	case AV_SAMPLE_FMT_U8P:
		pa = paUInt8;
		break;
	case AV_SAMPLE_FMT_S16:
	case AV_SAMPLE_FMT_S16P:
		pa = paInt16;
		break;
	case AV_SAMPLE_FMT_S32:
	case AV_SAMPLE_FMT_S32P:
		pa = paInt32;
		break;
	case AV_SAMPLE_FMT_FLT:
	case AV_SAMPLE_FMT_FLTP:
		pa = paFloat32;
		break;
	default:
		return AV_SAMPLE_FMT_NONE;
		// throw std::runtime_error("sample format not supported by portaudio: " + std::to_string(av));
	}

	if (av >= 5)
		pa |= paNonInterleaved;

	return pa;
}

void without_resampling(const char *const url)
{
	av::MediaFileReader format(url);
	const auto stream = format.find_best_stream(AVMEDIA_TYPE_AUDIO);
	av::StreamDecoder decoder(stream);

	const auto cdctx = decoder.cdctx();
	cdctx->request_sample_fmt = AV_SAMPLE_FMT_FLT;
	decoder.open();

	pa::PortAudio _;
	pa::Stream pa_stream(0, decoder.cdpar->ch_layout.nb_channels,
						 avsf2pasf(cdctx->sample_fmt),
						 decoder.cdpar->sample_rate,
						 paFramesPerBufferUnspecified);

	std::cout << "requested sample format: " << av_get_sample_fmt_name(cdctx->request_sample_fmt) << '\n'
			  << "codec: " << avcodec_get_name(decoder.codec->id) << '\n'
			  << "sample format: " << av_get_sample_fmt_name(cdctx->sample_fmt) << '\n';

	while (const auto packet = format.read_packet())
	{
		if (packet->stream_index != decoder.stream->index)
			continue;
		if (!decoder.send_packet(packet))
			break;
		while (const auto frame = decoder.receive_frame())
		{
			assert(cdctx->sample_fmt == frame->format);
			pa_stream.write(is_interleaved(cdctx->sample_fmt)
								? (void *)frame->extended_data[0]
								: (void *)frame->extended_data,
							frame->nb_samples);
		}
	}
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 2)
	{
		std::cerr << "media url required\n";
		return EXIT_FAILURE;
	}

	with_resampling(argv[1]);
	// without_resampling(argv[1]);
}
