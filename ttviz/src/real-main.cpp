#include "Main.hpp"

static int run_main(const int argc, const char *const *const argv)
{
	try
	{
		Main(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << "ttviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifndef _WIN32

int main(const int argc, const char *const *const argv)
{
	return run_main(argc, argv);
}

#else

#include <Windows.h>
#include <shellapi.h>
#include <stdexcept>
#include <vector>

static std::string wide_to_utf8(const wchar_t *wide)
{
	if (!wide)
		return {};
	const auto required = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
	if (required <= 0)
		throw std::runtime_error{"WideCharToMultiByte failed"};
	std::string utf8(static_cast<size_t>(required), '\0');
	if (WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8.data(), required, nullptr, nullptr) <= 0)
		throw std::runtime_error{"WideCharToMultiByte failed"};
	utf8.resize(static_cast<size_t>(required - 1));
	return utf8;
}

int main()
{
	int argc = 0;
	const auto wide_argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (!wide_argv)
		return run_main(__argc, __argv);

	std::vector<std::string> utf8_args;
	utf8_args.reserve(static_cast<size_t>(argc));
	for (int i = 0; i < argc; ++i)
		utf8_args.emplace_back(wide_to_utf8(wide_argv[i]));

	std::vector<const char *> c_args;
	c_args.reserve(utf8_args.size());
	for (const auto &arg : utf8_args)
		c_args.push_back(arg.c_str());

	LocalFree(wide_argv);

	return run_main(static_cast<int>(c_args.size()), c_args.data());
}

#endif
