#include <audioviz/util.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <iostream>

namespace audioviz::util
{

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

float weighted_max(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func,
	const float size_divisor) // generally the lower third of the frequency spectrum is considered bass
{
	/**
	 * TODO: rewrite weighted max function to have slightly lower weight as frequencies approach 20hz,
	 * aka the first spectrum bar/element in either data vector.
	 */

	const auto _weighted_max = [&](const auto begin, const auto end, const auto weight_start)
	{
		auto max_value = *begin;
		const auto total_distance = static_cast<float>(std::distance(weight_start, end));
		for (auto it = begin; it < end; ++it)
		{
			// weight each element before it is compared
			const auto unweighted = std::distance(it, end) / total_distance;

			// clang-format off
			const auto weight = (it < weight_start)
				? 1.f
				: (weight_func ? weight_func(unweighted) : unweighted);
			// clang-format on

			const auto value = *it * weight;

			if (value > max_value)
				max_value = value;
		}
		return max_value;
	};

	const auto begin = vec.begin();
	const auto amount = vec.size() / size_divisor;
	return _weighted_max(
		begin,
		begin + amount,		   // only the first 50% of the range will have full weight
		begin + (amount / 2)); // these are generally the strongest bass frequencies to the ear
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
	const auto cmd{
		"ffmpeg -v warning -i \"" + mediaPath + "\" -an -sn -map disp:attached_pic -c copy -f image2pipe -"};
	std::cout << __FUNCTION__ << ": running command: '" << cmd << "'\n";

	const auto pipe = popen(cmd.c_str(), POPEN_R_MODE);
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
		std::cerr << __FUNCTION__ << ": pclose returned: " << rc << '\n';
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

    std::cout << __FUNCTION__ << ": running command: '" << cmd << "'\n";
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        std::cerr << __FUNCTION__ << ": ffmpeg command failed with code " << rc << '\n';
        return {};
    }

    sf::Texture texture;
    if (!texture.loadFromFile(tmpFile)) {
        std::cerr << __FUNCTION__ << ": failed to load texture from " << tmpFile << '\n';
        std::remove(tmpFile.c_str());
        return {};
    }

    std::remove(tmpFile.c_str());
    return texture;
}
*/

} // namespace audioviz::util
