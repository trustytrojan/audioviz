#include "argparse.hpp"

using argparse::ArgumentParser;

struct Args : protected ArgumentParser
{
	Args(const int argc, const char *const *const argv);
};