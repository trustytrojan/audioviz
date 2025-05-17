#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include <audioviz/util.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/search_path.hpp>
#include <boost/log/trivial.hpp>

namespace audioviz::media
{

FfmpegCliBoostMedia::FfmpegCliBoostMedia(const std::string &url, const sf::Vector2u video_size)
	: FfmpegCliMedia{url, video_size},
	  video_buffer(4 * video_size.x * video_size.y)
{
	{ // read attached pic
		const auto &streams = _format.streams();
		if (const auto itr = std::ranges::find_if(
				streams, [](const auto &s) { return s->disposition & AV_DISPOSITION_ATTACHED_PIC; });
			itr != streams.cend())
		{
			const auto &stream = *itr;
			_attached_pic = {stream->attached_pic.data, stream->attached_pic.size};
		}
	}

	try // to find a video stream
	{
		const auto stream = _format.find_best_stream(AVMEDIA_TYPE_VIDEO);
		// we don't want to re-decode the attached pic stream
		if (!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
			_vstream = stream;
	}
	catch (const av::Error &e)
	{
		BOOST_LOG_TRIVIAL(info) << e.what() << '\n';
	}

	{ // create audio decoder
		std::vector<std::string> args{"-v", "warning"};
		if (url.contains("http"))
			args.insert(args.end(), {"-reconnect", "1"});
		args.insert(args.end(), {"-i", url, "-c:a", "pcm_f32le", "-f", "f32le", "-"});
		BOOST_LOG_TRIVIAL(debug) << "audio args: ";
		for (const auto &arg : args)
			BOOST_LOG_TRIVIAL(debug) << '\'' << arg << "' ";
		BOOST_LOG_TRIVIAL(debug) << '\n';
		audioc = bp::child{bp::search_path("ffmpeg"), args, bp::std_out > audio};
	}

	if (_vstream && video_size.x && video_size.y)
	{ // create video decoder
		std::vector<std::string> args{"-v", "warning", "-hwaccel", "auto"};
		if (url.contains("http"))
			args.insert(args.end(), {"-reconnect", "1"});
		args.insert(args.end(), {"-i", url});

		// this needs more work!!!!!!!!!!
		// need to determine whether vaapi works on the system before using vaapi
		// also change quoting for windows

#ifdef LINUX
		if (const auto vaapi_device = util::detect_vaapi_device(); !vaapi_device.empty())
			// clang-format off
			args.insert(args.end(), {
				"-vaapi_device", vaapi_device,

				// va-api hardware accelerated scaling!
				"-vf", "format=nv12,hwupload,scale_vaapi=" + std::to_string(video_size.x) + ':' + std::to_string(video_size.y) + ",hwdownload"
			});
		// clang-format on
		else
			args.insert(args.end(), {"-s", std::to_string(video_size.x) + "x" + std::to_string(video_size.y)});
#else
		args.insert(args.end(), {"-s", std::to_string(video_size.x) + "x" + std::to_string(video_size.y)});
#endif

		args.insert(args.end(), {"-pix_fmt", "rgba", "-f", "rawvideo", "-"});

		BOOST_LOG_TRIVIAL(debug) << "video args: ";
		for (const auto &arg : args)
			BOOST_LOG_TRIVIAL(debug) << '\'' << arg << "' ";
		BOOST_LOG_TRIVIAL(debug) << '\n';

		videoc = bp::child{bp::search_path("ffmpeg"), args, bp::std_out > video};
	}
}

FfmpegCliBoostMedia::~FfmpegCliBoostMedia()
{
	audio.close();
	video.close();
	audioc.wait();
	videoc.wait();
}

size_t FfmpegCliBoostMedia::read_audio_samples(float *const buf, const int samples)
{
	const auto bytes_read = audio.read(reinterpret_cast<char *>(buf), samples * sizeof(float));
	const auto floats_read = bytes_read / sizeof(float);
	BOOST_LOG_TRIVIAL(trace) << "bytes_read=" << bytes_read << " floats_read=" << floats_read
			  << '\n';
	return floats_read;
}

bool FfmpegCliBoostMedia::read_video_frame(sf::Texture &txr)
{
	if (!txr.resize(video_size))
		throw std::runtime_error{"texture resize failed!"};
	const auto bytes_to_read = 4 * video_size.x * video_size.y;
	int bytes_read = 0;
	while (bytes_read < bytes_to_read)
	{
		const auto _bytes_read =
			video.read(reinterpret_cast<char *>(video_buffer.data()) + bytes_read, bytes_to_read - bytes_read);
		if (!_bytes_read)
			return false;
		bytes_read += _bytes_read;
	}
	txr.update(video_buffer.data());
	return true;
}

} // namespace audioviz::media
