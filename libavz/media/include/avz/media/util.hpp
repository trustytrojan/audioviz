#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <optional>
#include <span>
#include <string>

namespace avz::util
{

#ifdef __linux__
std::string detect_vaapi_device();
#endif

std::optional<sf::Texture> getAttachedPicture(const std::string &mediaPath);

FILE *popen_utf8(const std::string &command, const char *mode);

int pclose_utf8(FILE *stream);

}