#include "audioviz/Layer.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_Layer()
{
	// clang-format off
	new_usertype<Layer>("Layer",
		"", sol::no_constructor,
		"set_orig_cb", &Layer::set_orig_cb,
		"set_fx_cb", &Layer::set_fx_cb,
		"set_auto_fx", &Layer::set_auto_fx,
		"add_drawable", [](Layer &self, const sf::Drawable *drawable)
		{
			self.drawables.emplace_back(drawable);
		},
		"add_effect", [](Layer &self, const fx::Effect *effect)
		{
			self.effects.emplace_back(effect);
		}
	);
	// clang-format on
}

} // namespace luaviz
