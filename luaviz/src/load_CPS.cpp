#include "audioviz/ParticleSystem.hpp"
#include <table.hpp>

using namespace audioviz;

namespace luaviz
{

void table::load_CPS()
{
	using CPS = ParticleSystem<Particle<sf::CircleShape>>;
	// clang-format off
	new_usertype<CPS>("CircleParticleSystem",
		"new", sol::constructors<CPS(int)>(),
		"new", sol::factories([](const sol::table &rect, const int particle_count)
		{
			return new CPS(table_to_intrect(rect), particle_count);
		}),
		"set_displacement_direction", &CPS::set_displacement_direction,
		"set_start_position", &CPS::set_start_side,
		"set_rect", &CPS::set_rect,
		"set_particle_count", &CPS::set_particle_count,
		sol::base_classes, sol::bases<sf::Drawable>()
	);
	// clang-format on
}

} // namespace luaviz
