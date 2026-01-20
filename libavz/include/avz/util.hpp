#pragma once

#include <SFML/Graphics.hpp>
#include <avz/fft/Interpolator.hpp>
#include <cstdio>
#include <optional>
#include <span>
#include <string>

namespace avz::util
{

sf::Color hsv2rgb(float h, const float s, const float v);
sf::Vector3f interpolate(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);
sf::Vector3f interpolate_and_reverse(float t, sf::Vector3f start_hsv, sf::Vector3f end_hsv);

inline const sf::BlendMode GreatAmazingBlendMode{sf::BlendMode::Factor::OneMinusDstColor, sf::BlendMode::Factor::One};

#ifdef __linux__
std::string detect_vaapi_device();
#endif

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath);

FILE *popen_utf8(const std::string &command, const char *mode);

int pclose_utf8(FILE *stream);

sf::String utf8_to_sf_string(const std::string &text);

inline int bin_index_from_freq(const int freq_hz, const int sample_rate_hz, const int bin_count)
{
	return freq_hz * bin_count / sample_rate_hz;
}

void spread_out(std::span<float> out, std::span<const float> in);
void extract_channel(std::span<float> out, std::span<const float> in, int num_channels, int channel);

void resample_spectrum(
	std::span<float> spectrum,
	std::span<const float> in_amps,
	int sample_rate_hz,
	int fft_size,
	float start_freq,
	float end_freq,
	avz::Interpolator &interpolator);

} // namespace avz::util
