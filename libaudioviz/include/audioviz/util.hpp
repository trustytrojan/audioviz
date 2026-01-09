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

/**
 * Returns the index of the weighted maximum within the bass frequencies of `vec`.
 * 
 * Analyzes the first `1 / size_divisor` elements (default: lowest ~28% for bass).
 * Within that range:
 * - First half gets full weight (1.0)
 * - Second half gets weighted by `weight_func(distance_to_end)` to emphasize the strongest bass
 * 
 * @param vec Input spectrum (typically from FrequencyAnalyzer)
 * @param weight_func Function to apply to distances (default: linear; use sqrtf for perceptual weighting)
 * @param size_divisor Divisor for spectrum range (default: 3.5 = ~28% of spectrum = bass range)
 * @return Index into original `vec` of the weighted-maximum bass bin
 */
size_t weighted_max_index(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func = {},
	const float size_divisor = 3.5f); // generally the lower third of the frequency spectrum is considered bass

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

struct DragResizeResult
{
	bool moved{};
	bool resized{};
	sf::IntRect rect{};
};

DragResizeResult imgui_drag_resize(sf::IntRect rect, const float handle_size = 8.f);

} // namespace audioviz::util
