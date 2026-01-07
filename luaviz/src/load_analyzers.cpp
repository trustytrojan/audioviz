#include "table.hpp"
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_AA_SA()
{
	using AA = AudioAnalyzer;
	using SA = StereoAnalyzer;
	// clang-format off
	new_usertype<AA>("AudioAnalyzer",
		"new", sol::constructors<AA(int)>(),
		"resize", &AA::resize
	);

	new_usertype<SA>("StereoAnalyzer",
		sol::base_classes, sol::bases<AA>(),
		"new", sol::constructors<SA()>(),
		"left_data", &SA::left_data,
		"right_data", &SA::right_data
	);
	// clang-format on
}

} // namespace luaviz
