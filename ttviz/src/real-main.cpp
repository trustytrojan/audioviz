#include "Main.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace bl = boost::log;

int main(const int argc, const char *const *const argv)
{
	bl::core::get()->set_filter(bl::trivial::severity >= bl::trivial::debug);

	try
	{
		Main(argc, argv);
	}
	catch (const std::exception &e)
	{
		BOOST_LOG_TRIVIAL(error) << "audioviz: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
}
