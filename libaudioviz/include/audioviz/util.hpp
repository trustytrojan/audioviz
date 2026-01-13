#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace audioviz::util
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);

/**
 * Returns the index of the maximum after applying a weight to each value.
 * Every index is weighted based on its normalized distance to the end of the span:
 * If `weight_func` is empty, the distance value itself is used as the weight (linear).
 */
size_t weighted_max_index(std::span<const float> values, const std::function<float(float)> &weight_func = {});

inline const sf::BlendMode GreatAmazingBlendMode{sf::BlendMode::Factor::OneMinusDstColor, sf::BlendMode::Factor::One};

#ifdef __linux__
std::string detect_vaapi_device();
#endif

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath);

FILE *popen_utf8(const std::string &command, const char *mode);
sf::String utf8_to_sf_string(const std::string &text);

struct DragResizeResult
{
	bool moved{};
	bool resized{};
	sf::IntRect rect{};
};

DragResizeResult imgui_drag_resize(sf::IntRect rect, const float handle_size = 8.f);

inline int bin_index_from_freq(const int freq_hz, const int sample_rate_hz, const int bin_count)
{
	return freq_hz * bin_count / sample_rate_hz;
}

void spread_out(std::span<float> out, std::span<const float> in);
void strided_copy(std::span<float> out, std::span<const float> in, int num_channels, int channel);

} // namespace audioviz::util
