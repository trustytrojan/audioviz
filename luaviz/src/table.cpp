#include "table.hpp"

namespace luaviz
{

table::table()
	: sol::table{}
{
	load_everything();
}

table::table(const sol::table &t)
	: sol::table{t}
{
	load_everything();
}

void table::load_everything()
{
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
	load_tb_conv();
}

} // namespace luaviz
