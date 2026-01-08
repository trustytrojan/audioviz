#include "audioviz/fft/FrequencyAnalyzer.hpp"
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

void table::load_FA()
{
	using FA = FrequencyAnalyzer;
	// clang-format off
	new_enum("WindowFunction",
		"NONE", nullptr,
		"HANNING", FA::WF_HANNING,
		"HAMMING", FA::WF_HAMMING,
		"BLACKMAN", FA::WF_BLACKMAN
	);

	new_enum("FrequencyScale",
		"LINEAR", FA::Scale::LINEAR,
		"LOG", FA::Scale::LOG,
		"NTH_ROOT", FA::Scale::NTH_ROOT
	);

	new_enum("InterpolationType",
		"CSPLINE", FA::InterpolationType::CSPLINE,
		"CSPLINE_HERMITE", FA::InterpolationType::CSPLINE_HERMITE,
		"LINEAR", FA::InterpolationType::LINEAR,
		"NONE", FA::InterpolationType::NONE
	);

	new_enum("AccumulationMethod", 
		"SUM", FA::AccumulationMethod::SUM,
		"MAX", FA::AccumulationMethod::MAX
	);

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
