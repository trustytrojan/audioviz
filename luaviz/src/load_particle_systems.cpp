#include "audioviz/ParticleSystem.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_particle_systems()
{
	using CPS = ParticleSystem<sf::CircleShape>;
	using RPS = ParticleSystem<sf::RectangleShape>;

	// clang-format off
	new_usertype<CPS::UpdateOptions>("ParticleSystemUpdateOptions",
		"calm_factor", sol::property(&CPS::UpdateOptions::calm_factor),
		"multiplier", sol::property(&CPS::UpdateOptions::multiplier),
		"displacement_func", sol::property(&CPS::UpdateOptions::displacement_func),
		"weight_func", sol::property(&CPS::UpdateOptions::weight_func)
	);

	new_usertype<CPS>("CircleParticleSystem",
		sol::base_classes, sol::bases<sf::Drawable>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return CPS(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &CPS::set_displacement_direction,
		"set_rect", &CPS::set_rect,
		"set_particle_count", &CPS::set_particle_count,
		"update", sol::overload(
			static_cast<void(CPS::*)(AudioAnalyzer &, int, int, const CPS::UpdateOptions &)>(&CPS::update),
			static_cast<void(CPS::*)(AudioAnalyzer &, int, int, int, const CPS::UpdateOptions &)>(&CPS::update),
			[](CPS &self, AudioAnalyzer &aa, int sample_rate_hz, int fft_size) { self.update(aa, sample_rate_hz, fft_size); },
			[](CPS &self, AudioAnalyzer &aa, int sample_rate_hz, int fft_size, int channel) { self.update(aa, sample_rate_hz, fft_size, channel); }
		),
		"set_particle_textures", &CPS::set_particle_textures,
		"set_debug_rect", &CPS::set_debug_rect,
		"set_color", sol::overload(
			&CPS::set_color,
			[](CPS &self, const sol::table &color) { self.set_color(table_to_color(color)); }
		)
	);

	new_usertype<RPS>("RectangleParticleSystem",
		sol::base_classes, sol::bases<sf::Drawable>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return RPS(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &RPS::set_displacement_direction,
		"set_rect", &RPS::set_rect,
		"set_particle_count", &RPS::set_particle_count,
		"update", sol::overload(
			static_cast<void(RPS::*)(AudioAnalyzer &, int, int, const RPS::UpdateOptions &)>(&RPS::update),
			[](RPS &self, AudioAnalyzer &aa, int sample_rate_hz, int fft_size) { self.update(aa, sample_rate_hz, fft_size); }
		),
		"set_particle_textures", &RPS::set_particle_textures,
		"set_color", sol::overload(
			&RPS::set_color,
			[](RPS &self, const sol::table &color) { self.set_color(table_to_color(color)); }
		)
	);
	// clang-format on
}

} // namespace luaviz
