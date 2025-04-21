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
	new_enum("ParticleSystemStartSide",
		"BOTTOM", CPS::StartSide::BOTTOM,
		"TOP", CPS::StartSide::TOP,
		"LEFT", CPS::StartSide::LEFT,
		"RIGHT", CPS::StartSide::RIGHT
	);

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
		"set_start_side", &CPS::set_start_side,
		"set_rect", &CPS::set_rect,
		"set_particle_count", &CPS::set_particle_count,
		"update", sol::overload(
			static_cast<void(CPS::*)(const fft::AudioAnalyzer &, const CPS::UpdateOptions &)>(&CPS::update),
			static_cast<void(CPS::*)(const std::vector<float> &, const CPS::UpdateOptions &)>(&CPS::update),
			[](CPS &self, const fft::AudioAnalyzer &aa) { self.update(aa); },
			[](CPS &self, const std::vector<float> &v) { self.update(v); }
		),
		"set_particle_textures", &CPS::set_particle_textures,
		"set_debug_rect", &CPS::set_debug_rect
	);

	new_usertype<RPS>("RectangleParticleSystem",
		sol::base_classes, sol::bases<sf::Drawable>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return RPS(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &RPS::set_displacement_direction,
		"set_start_side", &RPS::set_start_side,
		"set_rect", &RPS::set_rect,
		"set_particle_count", &RPS::set_particle_count,
		"update", sol::overload(
			static_cast<void(RPS::*)(const fft::AudioAnalyzer &, const RPS::UpdateOptions &)>(&RPS::update),
			[](RPS &self, const fft::AudioAnalyzer &aa) { self.update(aa); }
		),
		"set_particle_textures", &RPS::set_particle_textures
	);
	// clang-format on
}

} // namespace luaviz
