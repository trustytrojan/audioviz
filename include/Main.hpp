#pragma once

#include "audioviz.hpp"
#include "Args.hpp"

class Main : Args, public audioviz
{
	std::string ffmpeg_path;

public:
	Main(const int argc, const char *const *const argv);
	void start();
};