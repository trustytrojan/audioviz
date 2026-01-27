#include "util.hpp"
#include <avz/media/FfmpegPopenMedia.hpp>
#include <cstring>
#include <iostream>
#include <sstream>

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
		ss << "-vaapi_device " << vaapi_device << " -vf format=nv12,hwupload,scale_vaapi=" << scaled_width << ':'
		   << scaled_height << ",hwdownload ";
	}
	else
		ss << "-s " << scaled_width << 'x' << scaled_height << ' ';
#else
	// va-api probably doesn't exist on this platform, software scale instead
	ss << "-s " << scaled_width << 'x' << scaled_height << ' ';
#endif

	ss << "-pix_fmt rgba -f rawvideo -";

	const auto command = ss.str();
	if (!(video = util::popen_utf8(command, POPEN_R_MODE)))
		// non-fatal error, we can continue without video
		perror("[FfmpegPopenMedia::init_video] popen");
}

std::optional<std::vector<std::byte>> FfmpegPopenMedia::getAttachedPicture(const std::string &mediaPath)
{
	const auto cmd{"ffmpeg -v warning -i \"" + mediaPath + "\" -an -sn -map disp:attached_pic -c copy -f image2pipe -"};
	std::cerr << __func__ << ": running command: '" << cmd << "'\n";

	const auto pipe = util::popen_utf8(cmd, POPEN_R_MODE);
	if (!pipe)
	{
		std::cerr << __func__ << ": popen: " << strerror(errno) << '\n';
		return {};
	}

	std::vector<std::byte> buffer;
	while (!feof(pipe) && !ferror(pipe))
	{
		std::byte buf[4096];
		const auto bytesRead = fread(buf, 1, sizeof(buf), pipe);
		if (bytesRead > 0)
			buffer.insert(buffer.end(), buf, buf + bytesRead);
	}

	const auto status = pclose(pipe);
	if (status == -1)
	{
		std::cerr << __func__ << ": pclose: " << strerror(errno) << '\n';
		return {};
	}

#ifdef _WIN32
	// On Windows (including MinGW), _pclose/pclose_utf8 return the process
	// exit code directly (not a wait(2)-style status), so test for 0.
	if (status == 0)
		return buffer;

	std::cerr << __func__ << ": ffmpeg exited with " << status << '\n';
	return {};
#else
	// On POSIX, pclose returns a wait(2)-style status; use WIFEXITED/WEXITSTATUS.
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return buffer;

	std::cerr << __func__ << ": ffmpeg exited with " << WEXITSTATUS(status) << '\n';
	return {};
#endif
}

FfmpegPopenMedia::FfmpegPopenMedia(
	const std::string &url, const unsigned scaled_width, const unsigned scaled_height, const float start_time_sec)
	: Media{url},
	  scaled_width{scaled_width},
	  scaled_height{scaled_height},
	  metadata{url}
{
	_attached_pic = getAttachedPicture(url);
	init_audio(start_time_sec);
	if (has_video_stream() && scaled_width && scaled_height)
		init_video();
}

FfmpegPopenMedia::FfmpegPopenMedia(const std::string &url, float start_time_sec)
	: Media{url},
	  metadata{url}
{
	_attached_pic = getAttachedPicture(url);
	init_audio(start_time_sec);
}

FfmpegPopenMedia::~FfmpegPopenMedia()
{
	if (audio && pclose(audio) == -1)
		perror("[FfmpegPopenMedia::~FfmpegPopenMedia] audio: pclose");
	if (video && pclose(video) == -1)
		perror("[FfmpegPopenMedia::~FfmpegPopenMedia] video: pclose");
}

size_t FfmpegPopenMedia::read_audio_samples(float *const buf, const int samples)
{
	if (!audio)
		throw std::logic_error{"[FfmpegPopenMedia::read_audio_samples] no audio stream"};
	return fread(buf, sizeof(float), samples, audio);
}

bool FfmpegPopenMedia::read_video_frame(std::vector<std::byte> &buf)
{
	if (!video)
		throw std::logic_error{"[FfmpegPopenMedia::read_video_frame] no video stream available!"};

	const auto bytes_to_read{4 * scaled_width * scaled_height};
	buf.resize(bytes_to_read);

	if (fread(buf.data(), sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
		return false;

	return true;
}

} // namespace avz
