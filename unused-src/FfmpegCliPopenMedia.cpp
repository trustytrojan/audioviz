#include "media/FfmpegCliPopenMedia.hpp"
#include <iostream>

FfmpegCliPopenMedia::FfmpegCliPopenMedia(const std::string &url, const sf::Vector2u video_size)
	: FfmpegCliMedia{url, video_size}
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

	try // find video stream
	{
		const auto stream = _format.find_best_stream(AVMEDIA_TYPE_VIDEO);
		// we don't want to re-decode the attached pic stream
		if (!(stream->disposition & AV_DISPOSITION_ATTACHED_PIC))
			_vstream = stream;
	}
	catch (const av::Error &e)
	{
		std::cerr << e.what() << '\n';
	}

	{ // create audio decoder
		std::ostringstream ss;
		ss << "ffmpeg -v warning -hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

#ifdef _WIN32
		ss << "-i \"" << url << "\" ";
#else
		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";
#endif

		ss << "-f f32le - ";

		if (!(audio = popen(ss.str().c_str(), "r")))
			throw std::runtime_error{std::string{"popen: "} + strerror(errno)};
	}

	if (_vstream && video_size.x && video_size.y)
	{ // create video decoder
		std::ostringstream ss;
		ss << "ffmpeg ";
		ss << "-v warning ";
		ss << "-hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";

// this needs more work!!!!!!!!!!
// need to determine whether vaapi works on the system before using vaapi
// also change quoting for windows

#ifdef LINUX
		ss << "-vaapi_device /dev/dri/renderD129 ";
		ss << "-vf 'format=nv12,hwupload,scale_vaapi=" << video_size.x << ':' << video_size.y << ",hwdownload' ";
#else
		ss << "-s " << video_size.x << 'x' << video_size.y << ' ';
#endif

		ss << "-pix_fmt rgba ";
		ss << "-f rawvideo ";
		ss << "-";

		if (!(video = popen(ss.str().c_str(), "r")))
			perror("popen");
	}
}

FfmpegCliPopenMedia::~FfmpegCliPopenMedia()
{
	if (audio && pclose(audio) == -1)
		perror("pclose");
	if (video && pclose(video) == -1)
		perror("pclose");
}

size_t FfmpegCliPopenMedia::read_audio_samples(float *const buf, const int samples)
{
	if (!audio)
		throw std::logic_error{"no audio stream"};
	const auto samples_read = fread(buf, sizeof(float), samples, audio);
	if (const auto err = ferror(audio))
		std::cerr << "audio stream error: " << strerror(errno) << '\n';
	return samples_read;
}

bool FfmpegCliPopenMedia::read_video_frame(sf::Texture &txr)
{
	if (!video)
		throw std::runtime_error{"no video stream available!"};
	if (!txr.resize(video_size))
		throw std::runtime_error{"texture resize failed!"};
	const auto bytes_to_read = 4 * video_size.x * video_size.y;
	uint8_t buf[bytes_to_read];
	if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
	{
		if (const auto err = ferror(video))
			std::cerr << "video error: " << strerror(err) << '\n';
		return false;
	}
	txr.update(buf);
	return true;
}
