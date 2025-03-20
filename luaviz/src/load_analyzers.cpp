#include "table.hpp"
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_AA_SA()
{
	using AA = fft::AudioAnalyzer;
	using SA = fft::StereoAnalyzer;
	// clang-format off
	new_usertype<AA>("AudioAnalyzer",
		"new", sol::constructors<AA(int)>(),
		"resize", &AA::resize
	);

	new_usertype<SA>("StereoAnalyzer",
		"new", sol::constructors<SA()>(),
		sol::base_classes, sol::bases<AA>()
	);
	// clang-format on
}

} // namespace luaviz
