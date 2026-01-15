#pragma once

#include "Args.hpp"
#include "ttviz.hpp"

class Main
{
	const Args args;

	Main(const Main &) = delete;
	Main &operator=(const Main &) = delete;
	Main(Main &&) = delete;
	Main &operator=(Main &&) = delete;

	void configure_from_args(ttviz &viz);

public:
	Main(const int argc, const char *const *const argv);
};
