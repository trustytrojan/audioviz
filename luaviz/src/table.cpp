#include "table.hpp"
#include <audioviz/ColorSettings.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>
#include <audioviz/media/FfmpegCliBoostMedia.hpp>
#include <audioviz/util.hpp>

using namespace audioviz;

namespace luaviz
{

table::table(const sol::table &t)
	: sol::table{t}
{
	using FA = fft::FrequencyAnalyzer;
	using AA = fft::AudioAnalyzer;
	using SA = fft::StereoAnalyzer;
	using CS = ColorSettings;

#ifdef LINUX
	set("os", "linux");
#elifdef _WIN32
	set("os", "windows");
#endif

	load_FA();
	load_AA_SA();
	load_RT();
	load_Layer();
	load_particle_systems();
	load_CS();
	load_BSD();
	load_BSS();
	load_BSC();
	load_media();
	load_sf_types();
	load_Sprite();
	load_SMD();
	load_Base();
	load_fx();
}

} // namespace luaviz
