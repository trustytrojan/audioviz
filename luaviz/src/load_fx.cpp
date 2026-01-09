#include "audioviz/fx/Shake.hpp"
#include "table.hpp"
#include <audioviz/fx/Add.hpp>
#include <audioviz/fx/Alpha.hpp>
#include <audioviz/fx/Blur.hpp>
#include <audioviz/fx/Mult.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_fx()
{
	// clang-format off
	new_usertype<fx::PostProcessEffect>("PostProcessEffect",
		"", sol::no_constructor
	);

	new_usertype<fx::Add>("Add",
		"new", sol::constructors<fx::Add(float)>(),
		"addend", sol::property(&fx::Add::addend),
		sol::base_classes, sol::bases<fx::PostProcessEffect>()
	);

	new_usertype<fx::Mult>("Mult",
		"new", sol::constructors<fx::Mult(float)>(),
		"factor", sol::property(&fx::Mult::factor),
		sol::base_classes, sol::bases<fx::PostProcessEffect>()
	);

	new_usertype<fx::Blur>("Blur",
		"new", sol::constructors<fx::Blur(float, float, int)>(),
		"hrad", sol::property(&fx::Blur::hrad),
		"vrad", sol::property(&fx::Blur::vrad),
		"n_passes", sol::property(&fx::Blur::n_passes),
		sol::base_classes, sol::bases<fx::PostProcessEffect>()
	);

	new_usertype<fx::Alpha>("Alpha",
		"new", sol::constructors<fx::Alpha(float)>(),
		"alpha", sol::property(&fx::Alpha::alpha),
		sol::base_classes, sol::bases<fx::PostProcessEffect>()
	);

	set_function("Shake_setParameters", static_cast<void(*)(const AudioAnalyzer &, int, int, float)>(&fx::Shake::setParameters));
	set_function("Shake_getShader", [] { return std::ref(fx::Shake::getShader()); });
	// set("Shake", shake_tb);

	// create_named("Shake",
	// 	"setParameters", static_cast<void(*)(const AudioAnalyzer &, int, int, float)>(&fx::Shake::setParameters),
	// 	"getShader", &fx::Shake::getShader
	// );
	// clang-format on
}

} // namespace luaviz
