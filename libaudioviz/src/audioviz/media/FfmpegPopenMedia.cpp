#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>
#include <iostream>

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath)
{
	const auto cmd{
		"ffmpeg -v warning -i '" + mediaPath + "' -an -sn -map disp:attached_pic -c copy -f image2pipe pipe:1"};

	const auto pipe = popen(cmd.c_str(), "r");
	if (!pipe)
	{
		std::cerr << __FUNCTION__ << ": popen: " << strerror(errno) << '\n';
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

	switch (const auto rc = pclose(pipe))
	{
	case -1:
		std::cerr << __FUNCTION__ << ": pclose: " << strerror(errno) << '\n';
		return {};
	case 0:
		return {{buffer.data(), buffer.size()}};
	default:
		std::cerr << "ffmpeg failed with exit code " << rc << '\n';
		return {};
	}
}

namespace audioviz::media
{

FfmpegPopenMedia::FfmpegPopenMedia(const std::string &url, const sf::Vector2u video_size)
	: Media{url, video_size},
	  metadata{url}
{
	_attached_pic = getAttachedPicture(url);

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
			throw std::runtime_error{std::string{"audio: popen: "} + strerror(errno)};
	}

	if (has_video_stream() && video_size.x && video_size.y)
	{ // create video decoder
		std::ostringstream ss;
		ss << "ffmpeg -v warning -hwaccel auto ";

		if (url.contains("http"))
			ss << "-reconnect 1 ";

		if (url.contains('\''))
			ss << "-i \"" << url << "\" ";
		else
			ss << "-i '" << url << "' ";

#ifdef LINUX
		if (const auto vaapi_device{util::detect_vaapi_device()}; !vaapi_device.empty())
		{
			// use vaapi-accelerated scaling!
			ss << "-vaapi_device " << vaapi_device << " -vf format=nv12,hwupload,scale_vaapi=" << video_size.x << ':'
			   << video_size.y << ",hwdownload ";
		}
#else
		// vaapi probably doesnt exist on this platform, software scale instead
		ss << "-s " << video_size.x << 'x' << video_size.y << ' ';
#endif

		ss << "-pix_fmt rgba -f rawvideo -";

		if (!(video = popen(ss.str().c_str(), "r")))
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
		throw std::runtime_error{"no video stream available!"};
	if (!txr.resize(video_size))
		throw std::runtime_error{"texture resize failed!"};
	const auto bytes_to_read{4 * video_size.x * video_size.y};
	uint8_t buf[bytes_to_read];
	if (fread(buf, sizeof(uint8_t), bytes_to_read, video) < bytes_to_read)
		return false;
	txr.update(buf);
	return true;
}

} // namespace audioviz::media
