#include <avz/media/FfmpegPopenMedia.hpp>
#include <avz/media/util.hpp>
#include <cstring>
#include <iostream>

namespace avz
{

void FfmpegPopenMedia::init_audio(float start_time_sec)
{
	std::ostringstream ss;
	ss << "ffmpeg -v warning ";

	if (url.contains("http"))
		ss << "-reconnect 1 ";

	if (start_time_sec != 0)
		ss << "-ss " << start_time_sec << ' ';
	ss << "-i \"" << url << "\" ";
	ss << "-c:a pcm_f32le -f f32le - ";

	const auto command = ss.str();
	if (!(audio = util::popen_utf8(command, POPEN_R_MODE)))
		// fatal error: audio visualizers need audio...
		throw std::runtime_error{std::string{"[FfmpegPopenMedia::init_audio] popen: "} + strerror(errno)};
}

void FfmpegPopenMedia::init_video()
{
	std::ostringstream ss;
	ss << "ffmpeg -v warning -hwaccel auto ";

	if (url.contains("http"))
		ss << "-reconnect 1 ";

	ss << "-i \"" << url << "\" ";

	// from the ffmpeg docs: ’V’ only matches video streams which
	// are not attached pictures, video thumbnails or cover arts
	ss << "-map V ";

#ifdef __linux__
	if (const auto vaapi_device{util::detect_vaapi_device()}; !vaapi_device.empty())
	{
		// use vaapi-accelerated scaling!
		ss << "-vaapi_device " << vaapi_device << " -vf format=nv12,hwupload,scale_vaapi=" << scaled_video_size.x << ':'
		   << scaled_video_size.y << ",hwdownload ";
	}
	else
		ss << "-s " << scaled_video_size.x << 'x' << scaled_video_size.y << ' ';
#else
	// va-api probably doesn't exist on this platform, software scale instead
	ss << "-s " << scaled_video_size.x << 'x' << scaled_video_size.y << ' ';
#endif

	ss << "-pix_fmt rgba -f rawvideo -";

	const auto command = ss.str();
	if (!(video = util::popen_utf8(command, POPEN_R_MODE)))
		// non-fatal error, we can continue without video
		perror("[FfmpegPopenMedia::init_video] popen");
}

FfmpegPopenMedia::FfmpegPopenMedia(const std::string &url, const sf::Vector2u desired_video_size, float start_time_sec)
	: Media{url},
	  scaled_video_size{desired_video_size},
	  metadata{url}
{
	_attached_pic = util::getAttachedPicture(url);
	init_audio(start_time_sec);
	if (has_video_stream() && scaled_video_size.x && scaled_video_size.y)
		init_video();
}

FfmpegPopenMedia::FfmpegPopenMedia(const std::string &url, float start_time_sec)
	: Media{url},
	  metadata{url}
{
	_attached_pic = util::getAttachedPicture(url);
	init_audio(start_time_sec);
}

FfmpegPopenMedia::~FfmpegPopenMedia()
{
	if (audio && util::pclose_utf8(audio) == -1)
		perror("[FfmpegPopenMedia::~FfmpegPopenMedia] audio: pclose");
	if (video && util::pclose_utf8(video) == -1)
		perror("[FfmpegPopenMedia::~FfmpegPopenMedia] video: pclose");
}

size_t FfmpegPopenMedia::read_audio_samples(float *const buf, const int samples)
{
	if (!audio)
		throw std::logic_error{"[FfmpegPopenMedia::read_audio_samples] no audio stream"};
	return fread(buf, sizeof(float), samples, audio);
}

bool FfmpegPopenMedia::read_video_frame(sf::Texture &txr)
{
	if (!video)
		throw std::logic_error{"[FfmpegPopenMedia::read_video_frame] no video stream available!"};
	if (!txr.resize(scaled_video_size))
		throw std::runtime_error{"[FfmpegPopenMedia::read_video_frame] texture resize failed!"};
	const auto bytes_to_read{4 * scaled_video_size.x * scaled_video_size.y};

#ifdef _WIN32
	// windows doesnt like large stacks so we have to heap-allocate instead
	const auto buf = new uint8_t[bytes_to_read];
	if (!buf)
		throw std::runtime_error{"[FfmpegPopenMedia::read_video_frame] failed to allocate buffer"};
#else // most other things are probably unix based so... this is fine
	uint8_t buf[bytes_to_read];
#endif

	if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
		return false;
	txr.update(buf);

#ifdef _WIN32
	delete[] buf;
#endif

	return true;
}

} // namespace avz
