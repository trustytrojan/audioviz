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
	new_usertype<fx::Effect>("Effect",
		"", sol::no_constructor
	);

	new_usertype<fx::Add>("Add",
		"new", sol::constructors<fx::Add(float)>(),
		"addend", sol::property(&fx::Add::addend),
		sol::base_classes, sol::bases<fx::Effect>()
	);

	new_usertype<fx::Mult>("Mult",
		"new", sol::constructors<fx::Mult(float)>(),
		"factor", sol::property(&fx::Mult::factor),
		sol::base_classes, sol::bases<fx::Effect>()
	);

	new_usertype<fx::Blur>("Blur",
		"new", sol::constructors<fx::Blur(float, float, int)>(),
		"hrad", sol::property(&fx::Blur::hrad),
		"vrad", sol::property(&fx::Blur::vrad),
		"n_passes", sol::property(&fx::Blur::n_passes),
		sol::base_classes, sol::bases<fx::Effect>()
	);

	new_usertype<fx::Alpha>("Alpha",
		"new", sol::constructors<fx::Alpha(float)>(),
		"alpha", sol::property(&fx::Alpha::alpha),
		sol::base_classes, sol::bases<fx::Effect>()
	);
	// clang-format on
}

} // namespace luaviz
