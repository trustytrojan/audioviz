#include <argparse/argparse.hpp>

using argparse::ArgumentParser;

struct Args : ArgumentParser
{
	Args(const int argc, const char *const *const argv);
};