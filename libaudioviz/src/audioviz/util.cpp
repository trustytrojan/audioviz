#include <algorithm>
#include <audioviz/util.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#ifdef AUDIOVIZ_IMGUI
#include "imgui.h"
#endif

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
#else
	return ::popen(command.c_str(), mode);
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

		const auto status = ::pclose(pipe);
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

	switch (const auto rc = pclose(pipe))
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

#ifdef AUDIOVIZ_IMGUI
DragResizeResult imgui_drag_resize(sf::IntRect rect, const float handle_size)
{
	DragResizeResult out{false, false, rect};

	ImGuiViewport *vp = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
	const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
								   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
								   ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground |
								   ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::Begin("##audioviz_drag_layer", nullptr, flags);

	ImDrawList *dl = ImGui::GetForegroundDrawList();
	const ImVec2 vpp = vp->Pos;

	ImVec2 min{vpp.x + rect.position.x, vpp.y + rect.position.y};
	ImVec2 max{min.x + rect.size.x, min.y + rect.size.y};

	// dl->AddRect(min, max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 1.0f);

	struct Handle
	{
		ImVec2 a, b;
		enum Kind
		{
			Move,
			TL,
			TR,
			BL,
			BR,
			L,
			R,
			T,
			B
		} kind;
	};

	const float h = handle_size;
	const ImVec2 center{(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f};

	Handle handles[] = {
		{{min.x, min.y}, {max.x, max.y}, Handle::Move},
		{{min.x - h, min.y - h}, {min.x + h, min.y + h}, Handle::TL},
		{{max.x - h, min.y - h}, {max.x + h, min.y + h}, Handle::TR},
		{{min.x - h, max.y - h}, {min.x + h, max.y + h}, Handle::BL},
		{{max.x - h, max.y - h}, {max.x + h, max.y + h}, Handle::BR},
		{{min.x - h, center.y - h}, {min.x + h, center.y + h}, Handle::L},
		{{max.x - h, center.y - h}, {max.x + h, center.y + h}, Handle::R},
		{{center.x - h, min.y - h}, {center.x + h, min.y + h}, Handle::T},
		{{center.x - h, max.y - h}, {center.x + h, max.y + h}, Handle::B},
	};

	const ImVec2 delta = ImGui::GetIO().MouseDelta;

	ImGui::PushID(&rect);
	for (int i = 0; i < (int)(sizeof(handles) / sizeof(handles[0])); ++i)
	{
		ImGui::SetCursorScreenPos(handles[i].a);
		const ImVec2 size{handles[i].b.x - handles[i].a.x, handles[i].b.y - handles[i].a.y};
		ImGui::PushID(i);
		ImGui::InvisibleButton("##resize_handle", size);
		const bool active = ImGui::IsItemActive();
		ImGui::PopID();

		if (handles[i].kind == Handle::Move)
			dl->AddRect(handles[i].a, handles[i].b, IM_COL32(255, 255, 0, active ? 180 : 80));
		else
			dl->AddRectFilled(handles[i].a, handles[i].b, IM_COL32(255, 255, 0, active ? 180 : 80));

		if (!active)
			continue;

		switch (handles[i].kind)
		{
		case Handle::Move:
			out.rect.position.x += (int)delta.x;
			out.rect.position.y += (int)delta.y;
			out.moved = true;
			break;
		case Handle::TL:
			out.rect.position.x += (int)delta.x;
			out.rect.position.y += (int)delta.y;
			out.rect.size.x -= (int)delta.x;
			out.rect.size.y -= (int)delta.y;
			out.resized = true;
			break;
		case Handle::TR:
			out.rect.position.y += (int)delta.y;
			out.rect.size.x += (int)delta.x;
			out.rect.size.y -= (int)delta.y;
			out.resized = true;
			break;
		case Handle::BL:
			out.rect.position.x += (int)delta.x;
			out.rect.size.x -= (int)delta.x;
			out.rect.size.y += (int)delta.y;
			out.resized = true;
			break;
		case Handle::BR:
			out.rect.size.x += (int)delta.x;
			out.rect.size.y += (int)delta.y;
			out.resized = true;
			break;
		case Handle::L:
			out.rect.position.x += (int)delta.x;
			out.rect.size.x -= (int)delta.x;
			out.resized = true;
			break;
		case Handle::R:
			out.rect.size.x += (int)delta.x;
			out.resized = true;
			break;
		case Handle::T:
			out.rect.position.y += (int)delta.y;
			out.rect.size.y -= (int)delta.y;
			out.resized = true;
			break;
		case Handle::B:
			out.rect.size.y += (int)delta.y;
			out.resized = true;
			break;
		}
	}
	ImGui::PopID();

	ImGui::End();
	ImGui::PopStyleVar(3);

	// clamp to a minimum size of 1x1
	out.rect.size.x = std::max(out.rect.size.x, 1);
	out.rect.size.y = std::max(out.rect.size.y, 1);

	return out;
}
#endif

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

void strided_copy(std::span<float> out, std::span<const float> in, int num_channels, int channel)
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
	std::span<float> out,
	std::span<const float> in_amps,
	int sample_rate_hz,
	int fft_size,
	float start_freq,
	float end_freq,
	Interpolator &interpolator)
{
	const float bin_size = (float)sample_rate_hz / fft_size;
	interpolator.set_values(in_amps);

	const float bin_pos_start = (start_freq / bin_size);
	const float bin_pos_end = (end_freq / bin_size);
	const float bin_pos_step = (bin_pos_end - bin_pos_start) / std::max(1.0f, (float)out.size() - 1.0f);

	float current_bin_pos = bin_pos_start;
	const auto out_size = out.size();

	for (size_t i = 0; i < out_size; ++i)
	{
		out[i] = interpolator.sample(current_bin_pos);
		current_bin_pos += bin_pos_step;
	}
}

} // namespace audioviz::util
