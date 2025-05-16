#include <iostream>
#include "Main.hpp"

void * __gxx_personality_v0=0;
void * _Unwind_Resume =0;

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
