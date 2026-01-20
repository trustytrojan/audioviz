#include "avz/ParticleSystem.hpp"
#include <random>

static float randf(const float min, const float max)
{
	static std::mt19937 gen(std::random_device{}());
	return std::uniform_real_distribution<>(min, max)(gen);
}

namespace avz
{

void ParticleSystem::set_framerate(const int framerate)
{
	timestep_scale = framerate > 0 ? 60.f / framerate : 1.f;
	for (auto &p : particles)
		p.scaleVelocity(timestep_scale);
}

void ParticleSystem::update(const float additional_displacement)
{
	// negative Y so that it boosts the particles
	// use sqrt to dampen the effect, otherwise the particles go crazy
	const sf::Vector2f displacement{0.f, -sqrtf(rect.size.y * additional_displacement) * timestep_scale};

	for (auto &p : particles)
	{
		// let the particle move using its velocity
		p.updatePosition();

		// then apply additional displacement upward
		auto new_pos = p.getPosition() + displacement;
		const auto radius = p.getRadius();

		// make sure particles don't escape the rect
		if (new_pos.x >= (rect.position.x + rect.size.x))
			// teleport from right edge to left
			new_pos.x = rect.position.x + -radius;
		else if (new_pos.x + radius < 0)
			// teleport from left edge to right
			new_pos.x = (rect.position.x + rect.size.x);

		p.setPosition(new_pos);

		// alpha = sqrt(distance from top)
		auto [r, g, b, a] = p.getFillColor();
		a = sqrtf((new_pos.y - rect.position.y) / rect.size.y) * 255;
		p.setFillColor({r, g, b, a});

		// teleport back to bottom once it reaches the top
		if (new_pos.y <= rect.position.y)
			teleport_particle_opposite_side(p);
	}
}

void ParticleSystem::draw(sf::RenderTarget &target, const sf::RenderStates states) const
{
	for (const auto &p : particles)
		target.draw(p, states);
	if (debug_rect)
	{
		sf::RectangleShape r{sf::Vector2f{rect.size}};
		r.setFillColor(sf::Color::Transparent);
		r.setOutlineThickness(1);
		r.setOutlineColor(sf::Color::White);
		r.setPosition(sf::Vector2f{rect.position});
		sf::CircleShape c{5};
		c.setPosition(sf::Vector2f{rect.position});
		target.draw(r, states);
		target.draw(c, states);

		r.setOutlineColor(sf::Color::Red);
		c.setFillColor(sf::Color::Red);
		target.draw(r);
		target.draw(c);
	}
}

void ParticleSystem::set_rect(const sf::IntRect &rect)
{
	if (this->rect == rect)
		return;
	this->rect = rect;

	// Check each particle and reinitialize only those out of bounds
	for (auto &p : particles)
		if (!rect.contains((sf::Vector2i)p.getPosition()))
			teleport_particle_opposite_side(p);
}

void ParticleSystem::init_particle(Particle &p)
{
	// TODO: everything here should be configurable

	// these are going to be very small circles, don't need many points
	p.setPointCount(10);
	p.setRadius(randf(2, 5));

	// start some particles offscreen, so the beginning sequence feels less "sudden"
	// otherwise all of them come out at once and it looks bad
	p.setPosition({
		randf(rect.position.x, rect.position.x + rect.size.x),
		rect.position.y + rect.size.y * randf(1, 2),
	});

	const auto vx = randf(-0.5f, 0.5f) * timestep_scale;
	const auto vy = randf(-2.f, 0.f) * timestep_scale;
	p.setVelocity({vx, vy});
}

void ParticleSystem::init_particles()
{
	for (auto &p : particles)
		init_particle(p);
}

void ParticleSystem::teleport_particle_opposite_side(Particle &p)
{
	// same as init_particle() but without scaling Y by randf(1, 2)
	p.setPosition({
		randf(rect.position.x, rect.position.x + rect.size.x),
		rect.position.y + rect.size.y,
	});
}

} // namespace avz
