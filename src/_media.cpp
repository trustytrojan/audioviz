#include "audioviz.hpp"
#include <libavformat/avformat.h>

void audioviz::_media::init(const audioviz &viz)
{
	// if an attached pic is in the format, use it for bg and album cover
	// clang-format off
	if (const auto itr = std::ranges::find_if(_format.streams(),
			[](const auto &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
		itr != _format.streams().cend())
	// clang-format on
	{
		const auto &stream = *itr;
		attached_pic = {stream->attached_pic.data, stream->attached_pic.size};
	}

	try
	{
		const auto _s = _format.find_best_stream(AVMEDIA_TYPE_VIDEO);
		// we don't want to re-decode the attached pic stream
		if (!(_s->disposition & AV_DISPOSITION_ATTACHED_PIC))
		{
			_vstream = _s;
			_vdecoder.emplace(avcodec_find_decoder(_s->codecpar->codec_id)).open();
			_scaler.emplace(
				av::SwScaler::SrcDstArgs{
					av::nearest_multiple_8(_s->codecpar->width),
					_s->codecpar->height,
					(AVPixelFormat)_s->codecpar->format,
				},
				av::SwScaler::SrcDstArgs{viz.size.x, viz.size.y, AV_PIX_FMT_RGBA});
			_scaled_frame.emplace();
			_frame_queue.emplace();
		}
	}
	catch (const av::Error &e)
	{
		switch (e.errnum)
		{
		case AVERROR_STREAM_NOT_FOUND:
			std::cerr << "video stream not found\n";
			break;
		case AVERROR_DECODER_NOT_FOUND:
			std::cerr << "video decoder not found\n";
			break;
		default:
			throw;
		}
	}

	// audio decoding initialization
	if (!_astream->codecpar->ch_layout.order)
		// this check is necessary for .wav files with no channel order information
		_astream->codecpar->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
	rs_frame->ch_layout = _astream->codecpar->ch_layout;
	rs_frame->sample_rate = _astream->codecpar->sample_rate;
	rs_frame->format = AV_SAMPLE_FMT_FLT;
	_adecoder.copy_params(_astream->codecpar);
	_adecoder.open();

	// TODO: NEED TO EXPERIMENT WITH USING THE FFMPEG CLI INSTEAD OF CALLING LIBAV
	// IT MAY BE THE ONLY WAY TO AVOID THE MESS BELOW.

	// some formats/codecs have bad packet durations.
	// no idea what is causing this. as of right now mp3 has a workaround,
	// but anything else might end playback too early.
	switch (_adecoder->codec_id)
	{
	case AV_CODEC_ID_MP3:
		_format.seek_file(-1, 1, 1, 1, AVSEEK_FLAG_FRAME);
		break;
	default:
		break;
	}
}

void audioviz::_media::decode(audioviz &viz)
{
	// while we don't have enough audio samples
	while ((int)viz.audio_buffer.size() < 2 * viz.fft_size)
	{
		const auto packet = _format.read_packet();

		if (!packet)
		{
			std::cerr << "packet is null; format probably reached eof\n";
			return;
		}

		if (packet->stream_index == _astream->index)
		{
			std::cerr << "\e[2K\raudio: " << (packet->pts * av_q2d(_astream->time_base)) << " / " << _astream.duration_sec();

			if (!_adecoder.send_packet(packet))
			{
				std::cerr << "audio decoder has been flushed\n";
				continue;
			}

			while (const auto frame = _adecoder.receive_frame())
			{
				_resampler.convert_frame(rs_frame.get(), frame);
				const auto data = reinterpret_cast<const float *>(rs_frame->extended_data[0]);
				const auto nb_floats = 2 * rs_frame->nb_samples;
				viz.audio_buffer.insert(viz.audio_buffer.end(), data, data + nb_floats);
			}
		}
		else if (_vstream && packet->stream_index == _vstream->get()->index)
		{
			// we can access all of the video-related optionals in this block

			if (!_vdecoder->send_packet(packet))
			{
				std::cerr << "video decoder has been flushed\n";
				continue;
			}

			while (const auto frame = _vdecoder->receive_frame())
			{
				_scaler->scale_frame(_scaled_frame->get(), frame);
				// put a frame into an sf::Texture, then put that into a frame queue.
				// we are going to be reading more packets than usual since
				// we need more audio samples than is provided by one audio packet.

				// create new texture object in-place at the end of the list
				_frame_queue->emplace_back(sf::Vector2u{_scaled_frame->get()->width, _scaled_frame->get()->height});
				// if (!_frame_queue->back().create({_scaled_frame->get()->width, _scaled_frame->get()->height}))
				// throw std::runtime_error("failed to create texture!");
				_frame_queue->back().update(_scaled_frame->get()->data[0]);
			}
		}
	}
}
