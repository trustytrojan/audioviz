#include <argparse/argparse.hpp>

using argparse::ArgumentParser;
using uint = unsigned int;

struct Args : ArgumentParser
{
	Args(const int argc, const char *const *const argv);
};