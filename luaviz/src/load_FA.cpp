#include "audioviz/fft/FrequencyAnalyzer.hpp"
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

void table::load_FA()
{
	using FA = fft::FrequencyAnalyzer;
	// clang-format off
	new_usertype<FA>("FrequencyAnalyzer",
		"new", sol::constructors<FA(int)>(),
		"set_fft_size", &FA::set_fft_size,
		"set_interp_type", &FA::set_interp_type,
		"set_window_func", &FA::set_window_func,
		"set_accum_method", &FA::set_accum_method,
		"set_scale", &FA::set_scale,
		"set_nth_root", &FA::set_nth_root
	);
	// clang-format on
}

} // namespace luaviz
