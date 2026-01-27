#pragma once

#include <cstdio>
#include <string>

namespace avz::util
{

#ifdef __linux__
std::string detect_vaapi_device();
#endif

FILE *popen_utf8(const std::string &command, const char *mode);

} // namespace avz::util
