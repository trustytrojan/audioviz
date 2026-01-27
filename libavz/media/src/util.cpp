#include "util.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
std::wstring utf8_to_wide(const std::string &str)
{
	if (str.empty())
		return {};
	const auto required = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	if (required == 0)
		return {};
	std::wstring wide(static_cast<size_t>(required), L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide.data(), required) == 0)
		return {};
	wide.resize(static_cast<size_t>(required - 1));
	return wide;
}
#endif

namespace avz::util
{

FILE *popen_utf8(const std::string &command, const char *mode)
{
#ifdef _WIN32
	const auto wide_command = utf8_to_wide(command);
	const auto wide_mode = utf8_to_wide(mode ? std::string{mode} : std::string{"r"});
	if (!wide_command.empty() && !wide_mode.empty())
		return _wpopen(wide_command.c_str(), wide_mode.c_str());
	return _popen(command.c_str(), mode ? mode : "r");
#else
	return popen(command.c_str(), mode);
#endif
}

#ifdef __linux__
std::string detect_vaapi_device()
{
	for (const auto &e : std::filesystem::directory_iterator{"/dev/dri"})
	{
		const auto &path = e.path();
		if (!path.filename().string().starts_with("renderD"))
			continue;
		std::cerr << __func__ << ": testing " << path << '\n';

		// Build the ffmpeg command
		const auto cmd{
			"ffmpeg -v warning -vaapi_device " + path.string() +
			" -f lavfi -i testsrc=1280x720:d=1 -vf format=nv12,hwupload,scale_vaapi=640:640 -c:v "
			"h264_vaapi -f null - 2>&1"};

		const auto pipe = popen(cmd.c_str(), "r");
		if (!pipe)
		{
			std::cerr << __func__ << ": popen: " << strerror(errno) << '\n';
			continue;
		}

		const auto status = pclose(pipe);
		if (status == -1)
		{
			std::cerr << __func__ << ": pclose: " << strerror(errno) << '\n';
			continue;
		}

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			std::cerr << __func__ << ": success, returning " << path << '\n';
			return path.string();
		}
	}

	std::cerr << __func__ << ": failed to find device\n";
	return {};
}
#endif

/*
This function is for handling cases where a media file has the coverart/thumbnail inside
an *attachment* stream, but this is not the same as a *video* stream. To handle this we use
the -dump_attachment:t option, which must write to a file on disk, and then we can call
sf::Texture::loadFromFile on that file.

Not important, since I mostly care about MP3s, however this is how MKVs are spit out when
calling yt-dlp with --embed-thumbnail, and the highest quality audio/video streams have vp9 or opus.

std::optional<sf::Texture> getAttachedPictureViaDump(const std::string &mediaPath)
{
	// Use ffmpeg to dump the first attachment (usually cover art) to a temp file
	const std::string tmpFile = "cover_attachment_tmp";
	const std::string cmd = "ffmpeg -v warning -y -dump_attachment:t=" + tmpFile + " -i \"" + mediaPath + "\"";

	std::cout << __func__ << ": running command: '" << cmd << "'\n";
	int rc = std::system(cmd.c_str());
	if (rc != 0) {
		std::cerr << __func__ << ": ffmpeg command failed with code " << rc << '\n';
		return {};
	}

	sf::Texture texture;
	if (!texture.loadFromFile(tmpFile)) {
		std::cerr << __func__ << ": failed to load texture from " << tmpFile << '\n';
		std::remove(tmpFile.c_str());
		return {};
	}

	std::remove(tmpFile.c_str());
	return texture;
}
*/

} // namespace avz::util
