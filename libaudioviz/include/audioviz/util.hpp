#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace audioviz::util
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);

float weighted_max(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func = {},
	const float size_divisor = 3.5f); // generally the lower third of the frequency spectrum is considered bass

inline const sf::BlendMode GreatAmazingBlendMode{sf::BlendMode::Factor::OneMinusDstColor, sf::BlendMode::Factor::One};

#ifdef __linux__
std::string detect_vaapi_device();
#endif

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath);

FILE *popen_utf8(const std::string &command, const char *mode);
sf::String utf8_to_sf_string(const std::string &text);

#ifdef AUDIOVIZ_IMGUI
struct DragResizeResult
{
	bool moved{};
	bool resized{};
	sf::IntRect rect{};
};

DragResizeResult imgui_drag_resize(sf::IntRect rect, const float handle_size = 8.f);
#endif

} // namespace audioviz::util
