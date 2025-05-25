#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>
#include <iostream>

#ifdef _WIN32
#define POPEN_MODE "rb"
#else
#define POPEN_MODE "r"
#endif

namespace audioviz
{

FfmpegPopenMedia::FfmpegPopenMedia(const std::string &url, const sf::Vector2u video_size)
	: Media{url, video_size},
	  metadata{url}
{
	_attached_pic = util::getAttachedPicture(url);

	{ // create audio decoder
		std::ostringstream ss;
		ss << "ffmpeg -v warning ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

		ss << "-i \"" << url << "\" ";
		ss << "-f f32le - ";

		if (!(audio = popen(ss.str().c_str(), POPEN_MODE)))
			// fatal error: audio visualizers need audio...
			throw std::runtime_error{std::string{"audio: popen: "} + strerror(errno)};
	}

	if (has_video_stream() && video_size.x && video_size.y)
	{ // create video decoder
		std::ostringstream ss;
		ss << "ffmpeg -v warning -hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

		ss << "-i \"" << url << "\" ";

		// from the ffmpeg docs: ’V’ only matches video streams which are not attached pictures, video thumbnails or cover arts
		ss << "-map V ";

#ifdef __linux__
		if (const auto vaapi_device{util::detect_vaapi_device()}; !vaapi_device.empty())
		{
			// use vaapi-accelerated scaling!
			ss << "-vaapi_device " << vaapi_device << " -vf format=nv12,hwupload,scale_vaapi=" << video_size.x << ':'
			   << video_size.y << ",hwdownload ";
		}
		else
			ss << "-s " << video_size.x << 'x' << video_size.y << ' ';
#else
		// va-api probably doesn't exist on this platform, software scale instead
		ss << "-s " << video_size.x << 'x' << video_size.y << ' ';
#endif

		ss << "-pix_fmt rgba -f rawvideo -";

		if (!(video = popen(ss.str().c_str(), POPEN_MODE)))
			// non-fatal error, we can continue without video
			perror("video: popen");
	}
}

FfmpegPopenMedia::~FfmpegPopenMedia()
{
	if (audio && pclose(audio) == -1)
		perror("audio: pclose");
	if (video && pclose(video) == -1)
		perror("video: pclose");
}

size_t FfmpegPopenMedia::read_audio_samples(float *const buf, const int samples)
{
	if (!audio)
		throw std::logic_error{"no audio stream"};
	const auto samples_read = fread(buf, sizeof(float), samples, audio);
	return samples_read;
}

bool FfmpegPopenMedia::read_video_frame(sf::Texture &txr)
{
	if (!video)
		throw std::runtime_error{"[FfmpegPopenMedia::read_video_frame] no video stream available!"};
	if (!txr.resize(video_size))
		throw std::runtime_error{"[FfmpegPopenMedia::read_video_frame] texture resize failed!"};
	const auto bytes_to_read{4 * video_size.x * video_size.y};

#ifdef unix
	uint8_t buf[bytes_to_read];
#elifdef _WIN32
	// windows doesnt like large stacks so we have to heap-allocate instead
	const auto buf = new uint8_t[bytes_to_read];
	if (!buf)
		throw std::runtime_error{"[FfmpegPopenMedia::read_video_frame] failed to allocate buffer"};
#endif

	if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
		return false;
	txr.update(buf);

#ifdef _WIN32
	delete[] buf;
#endif

	return true;
}

} // namespace audioviz::media
