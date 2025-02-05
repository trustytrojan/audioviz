#include <iostream>
#include "Main.hpp"

int main(const int argc, const char *const *const argv)
{
	try
	{
		Main(argc, argv);
	}
#ifdef AUDIOVIZ_LUA
	catch (sol::error)
	{
		return EXIT_FAILURE;
	}
#endif
	catch (const std::exception &e)
	{
		std::cerr << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
