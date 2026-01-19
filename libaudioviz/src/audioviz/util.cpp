#include <algorithm>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/util.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

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

#elifdef __APPLE__

#include <crt_externs.h>
#include <mutex>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>


// Track pid for each FILE* so pclose can wait on the correct process
namespace
{
std::mutex popen_map_mutex;
std::unordered_map<FILE *, pid_t> popen_pid_map;
} // namespace

#endif

namespace audioviz::util
{

FILE *popen_utf8(const std::string &command, const char *mode)
{
#ifdef _WIN32
	const auto wide_command = utf8_to_wide(command);
	const auto wide_mode = utf8_to_wide(mode ? std::string{mode} : std::string{"r"});
	if (!wide_command.empty() && !wide_mode.empty())
		return _wpopen(wide_command.c_str(), wide_mode.c_str());
	return _popen(command.c_str(), mode ? mode : "r");
#elif defined(__APPLE__)
	// On macOS, use posix_spawn instead of popen to avoid fork-after-threads issues
	// that cause "mutex lock failed: Invalid argument" when SFML's mutexes are initialized

	const bool is_read = (mode && mode[0] == 'r');
	int pipefds[2];

	if (pipe(pipefds) == -1)
		return nullptr;

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);

	if (is_read)
	{
		// Read mode: redirect child stdout to pipe write end
		posix_spawn_file_actions_adddup2(&actions, pipefds[1], STDOUT_FILENO);
		posix_spawn_file_actions_adddup2(&actions, pipefds[1], STDERR_FILENO);
		posix_spawn_file_actions_addclose(&actions, pipefds[0]);
		posix_spawn_file_actions_addclose(&actions, pipefds[1]);
	}
	else
	{
		// Write mode: redirect child stdin from pipe read end
		posix_spawn_file_actions_adddup2(&actions, pipefds[0], STDIN_FILENO);
		posix_spawn_file_actions_addclose(&actions, pipefds[0]);
		posix_spawn_file_actions_addclose(&actions, pipefds[1]);
	}

	// Build argv for "/bin/sh -c command"
	char *argv[] = {const_cast<char *>("sh"), const_cast<char *>("-c"), const_cast<char *>(command.c_str()), nullptr};

	pid_t pid;
	int rc = posix_spawnp(&pid, "sh", &actions, nullptr, argv, *_NSGetEnviron());
	posix_spawn_file_actions_destroy(&actions);

	if (rc != 0)
	{
		close(pipefds[0]);
		close(pipefds[1]);
		errno = rc;
		return nullptr;
	}

	// Close unused end in parent
	int parent_fd = is_read ? pipefds[0] : pipefds[1];
	int unused_fd = is_read ? pipefds[1] : pipefds[0];
	close(unused_fd);

	FILE *fp = fdopen(parent_fd, mode);
	if (!fp)
	{
		close(parent_fd);
		// Child is still running, wait for it to avoid zombie
		waitpid(pid, nullptr, 0);
		return nullptr;
	}

	// Store pid for later retrieval by our custom pclose
	{
		std::lock_guard<std::mutex> lock(popen_map_mutex);
		popen_pid_map[fp] = pid;
	}

	return fp;
#else
	return ::popen(command.c_str(), mode);
#endif
}

int pclose_utf8(FILE *stream)
{
#ifdef __APPLE__
	if (!stream)
	{
		errno = EINVAL;
		return -1;
	}

	pid_t pid;
	{
		std::lock_guard<std::mutex> lock(popen_map_mutex);
		auto it = popen_pid_map.find(stream);
		if (it == popen_pid_map.end())
		{
			// Not one of our spawned processes, try standard pclose
			return pclose(stream);
		}
		pid = it->second;
		popen_pid_map.erase(it);
	}

	// Close the FILE* stream
	if (fclose(stream) != 0)
		return -1;

	// Wait for child process
	int status;
	if (waitpid(pid, &status, 0) == -1)
		return -1;

	return status;
#elifdef _WIN32
	return _pclose(stream);
#else
	return ::pclose(stream);
#endif
}

sf::String utf8_to_sf_string(const std::string &text)
{
	return sf::String::fromUtf8(text.begin(), text.end());
}

sf::Color hsv2rgb(float h, const float s, const float v)
{
	if (s < 0 || s > 1 || v < 0 || v > 1)
		throw std::invalid_argument("s and v should be in the range [0, 1]");

	// Normalize h to the range [0, 1]
	h = std::fmod(h, 1);

	// Ensure h is not negative
	if (h < 0)
		h += 1;

	float r, g, b;

	if (s == 0)
		r = g = b = v;
	else
	{
		float var_h = h * 6;
		if (var_h == 6)
			var_h = 0;

		int var_i = var_h;
		float var_1 = v * (1 - s);
		float var_2 = v * (1 - s * (var_h - var_i));
		float var_3 = v * (1 - s * (1 - (var_h - var_i)));

		switch (var_i)
		{
		case 0:
			r = v;
			g = var_3;
			b = var_1;
			break;
		case 1:
			r = var_2;
			g = v;
			b = var_1;
			break;
		case 2:
			r = var_1;
			g = v;
			b = var_3;
			break;
		case 3:
			r = var_1;
			g = var_2;
			b = v;
			break;
		case 4:
			r = var_3;
			g = var_1;
			b = v;
			break;
		case 5:
			r = v;
			g = var_1;
			b = var_2;
			break;
		default:
			throw std::logic_error("impossible!!!");
		}
	}

	return {r * 255, g * 255, b * 255};
}

sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv)
{
	const auto [h1, s1, v1] = start_hsv;
	const auto [h2, s2, v2] = end_hsv;

	float h = h1 + std::fmod(t, 1.0) * (h2 - h1);
	float s = s1 + std::fmod(t, 1.0) * (s2 - s1);
	float v = v1 + std::fmod(t, 1.0) * (v2 - v1);

	return {h, s, v};
}

sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv)
{
	const auto [h1, s1, v1] = start_hsv;
	const auto [h2, s2, v2] = end_hsv;
	float reversed_t = std::abs(std::sin(t));
	float h = h1 + reversed_t * (h2 - h1);
	float s = s1 + reversed_t * (s2 - s1);
	float v = v1 + reversed_t * (v2 - v1);
	return {h, s, v};
}

size_t weighted_max_index(std::span<const float> values, const std::function<float(float)> &weight_func)
{
	if (values.empty())
		throw std::invalid_argument{"weighted_max_index: empty span"};
	if (values.size() == 1)
		return 0;

	const auto size = values.size();
	size_t max_index{};
	float max_value{values[0]};

	for (size_t i = 1; i < size; ++i)
	{
		const float distance_to_end = static_cast<float>((size - 1) - i) / static_cast<float>(size - 1); // 1..0
		const float weight = weight_func ? weight_func(distance_to_end) : distance_to_end;
		const float value = values[i] * weight;
		if (value > max_value)
		{
			max_value = value;
			max_index = i;
		}
	}

	return max_index;
}

#ifdef __linux__
std::string detect_vaapi_device()
{
	for (const auto &e : std::filesystem::directory_iterator("/dev/dri"))
	{
		const auto &path = e.path();
		if (!path.filename().string().starts_with("renderD"))
			continue;
		std::cerr << "detect_vaapi_device: testing " << path << '\n';

		// Build the ffmpeg command
		const auto cmd{
			"ffmpeg -v warning -vaapi_device " + path.string() +
			" -f lavfi -i testsrc=1280x720:d=1 -vf format=nv12,hwupload,scale_vaapi=640:640 -c:v "
			"h264_vaapi -f null - 2>&1"};

		const auto pipe = popen(cmd.c_str(), "r");
		if (!pipe)
		{
			std::cerr << "detect_vaapi_device: popen failed for " << path << '\n';
			continue;
		}

		const auto status = pclose(pipe);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			std::cerr << "detect_vaapi_device: success, returning " << path << '\n';
			return path.string();
		}
	}

	std::cerr << "detect_vaapi_device: failed to find device\n";
	return {};
}
#endif

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath)
{
	const auto cmd{"ffmpeg -v warning -i \"" + mediaPath + "\" -an -sn -map disp:attached_pic -c copy -f image2pipe -"};
	// std::cout << __func__ << ": running command: '" << cmd << "'\n"; // this is going to display garbage ascii

	const auto pipe = popen_utf8(cmd, POPEN_R_MODE);
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

	switch (const auto rc = pclose_utf8(pipe))
	{
	case -1:
		std::cerr << __func__ << ": pclose: " << strerror(errno) << '\n';
		return {};
	case 0:
		return {{buffer.data(), buffer.size()}};
	default:
		std::cerr << __func__ << ": pclose returned: " << rc << '\n';
		return {};
	}
}

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

void spread_out(std::span<float> out, std::span<const float> in)
{
	assert(out.size() >= in.size());
	const auto increment = out.size() / in.size();

	auto *__restrict const out_ptr = out.data();
	const auto *__restrict const in_ptr = in.data();

#pragma GCC ivdep
	for (size_t i = 0; i < in.size(); ++i)
		out_ptr[i * increment] = in_ptr[i];
}

void extract_channel(std::span<float> out, std::span<const float> in, int num_channels, int channel)
{
	assert(num_channels > 0);
	assert(out.size() * num_channels == in.size());

	auto *__restrict const out_ptr = out.data();
	const auto *__restrict const in_ptr = in.data();

#pragma GCC ivdep
	for (size_t i = 0; i < out.size(); ++i)
		out_ptr[i] = in_ptr[i * num_channels + channel];
}

void resample_spectrum(
	std::span<float> spectrum,
	std::span<const float> in_amps,
	int sample_rate_hz,
	int fft_size,
	float start_freq,
	float end_freq,
	Interpolator &interpolator)
{
	interpolator.set_values(in_amps);

	const float bin_size = (float)sample_rate_hz / fft_size;
	const float bin_pos_start = (start_freq / bin_size);
	const float bin_pos_end = (end_freq / bin_size);
	const float bin_pos_step = (bin_pos_end - bin_pos_start) / std::max(1.0f, (float)spectrum.size() - 1.0f);

	float current_bin_pos = bin_pos_start;
	const auto out_size = spectrum.size();

#pragma GCC ivdep
	for (size_t i = 0; i < out_size; ++i)
	{
		spectrum[i] = interpolator.sample(current_bin_pos);
		current_bin_pos += bin_pos_step;
	}
}

} // namespace audioviz::util
