#include "audioviz/fft/FrequencyAnalyzer.hpp"
#include "table.hpp"

using namespace audioviz;

namespace luaviz
{

void table::load_FA()
{
	using FA = fft::FrequencyAnalyzer;
	// clang-format off
	new_enum("WindowFunction",
		"NONE", FA::WF_NONE,
		"HANNING", FA::WF_HANNING,
		"HAMMING", FA::WF_HAMMING,
		"BLACKMAN", FA::WF_BLACKMAN
	);

	new_enum("FrequencyScale",
		"LINEAR", FA::Scale::LINEAR,
		"LOG", FA::Scale::LOG,
		"NTH_ROOT", FA::Scale::NTH_ROOT
	);

	new_usertype<FA>("FrequencyAnalyzer",
		"new", sol::constructors<FA(int)>(),
		"set_fft_size", &FA::set_fft_size,
		"set_interp_type", [](FA &self, const std::string &type)
		{
			if (type == "cspline")
				self.set_interp_type(FA::InterpolationType::CSPLINE);
			else if (type == "cspline_hermite")
				self.set_interp_type(FA::InterpolationType::CSPLINE_HERMITE);
			else if (type == "linear")
				self.set_interp_type(FA::InterpolationType::LINEAR);
			else if (type == "none")
				self.set_interp_type(FA::InterpolationType::NONE);
		},
		"set_window_func", &FA::set_window_func,
		"set_accum_method", [](FA &self, const std::string &am)
		{
			if (am == "sum")
				self.set_accum_method(FA::AccumulationMethod::SUM);
			else if (am == "max")
				self.set_accum_method(FA::AccumulationMethod::MAX);
		},
		"set_scale", &FA::set_scale,
		"set_nth_root", &FA::set_nth_root
	);
	// clang-format on
}

} // namespace luaviz
