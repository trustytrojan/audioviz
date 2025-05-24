#include "Main.hpp"

int main(const int argc, const char *const *const argv)
{
	try
	{
		Main(argc, argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
