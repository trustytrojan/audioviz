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
		"add_draw", sol::overload(
			[](Layer &self, const sf::Drawable *const drawable, const sf::RenderStates *const rs)
			{
				self.add_draw({*drawable, *rs});
			},
			[](Layer &self, const sf::Drawable *const drawable)
			{
				self.add_draw({*drawable});
			}
		),
		"add_effect", [](Layer &self, fx::PostProcessEffect *const effect)
		{
			self.add_effect(effect);
		}
	);
	// clang-format on
}

} // namespace luaviz
