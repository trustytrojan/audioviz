#include <avz/gfx/ParticleSystem.hpp>
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
		p.applyTimescale(timestep_scale);
}

void ParticleSystem::update(const float additional_displacement)
{
	// negative Y so that it boosts the particles up
	// use sqrt to dampen the effect, otherwise the particles go crazy
	const sf::Vector2f displacement{0, -std::sqrt(rect.size.y * additional_displacement) * timestep_scale};

	const float left_edge = rect.position.x;
	const float right_edge = rect.position.x + rect.size.x;

	for (auto &p : particles)
	{
		// let the particle move using its velocity
		p.updatePosition();

		// then apply additional displacement upward
		auto new_pos = p.getPosition() + displacement;
		const auto radius = p.getRadius();

		// make sure particles don't escape the rect
		// Wrapping Right to Left
		if (new_pos.x - radius > right_edge)
			new_pos.x = left_edge - radius;
		// Wrapping Left to Right
		else if (new_pos.x + radius < left_edge)
			new_pos.x = right_edge + radius;

		p.setPosition(new_pos);

		if (fade_out)
		{
			auto [r, g, b, a] = p.getFillColor();
			// alpha = sqrt(distance from top)
			a = std::sqrt((new_pos.y - rect.position.y) / rect.size.y) * 255;
			p.setFillColor({r, g, b, a});
		}

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
	p.setRadius(randf(1, 5));

	// start some particles offscreen, so the beginning sequence feels less "sudden"
	// otherwise all of them come out at once and it looks bad
	p.setPosition({
		randf(rect.position.x, rect.position.x + rect.size.x),
		start_offscreen ? (rect.position.y + rect.size.y * randf(1, 2))
						: randf(rect.position.y, rect.position.y + rect.size.y),
	});

	const auto vx = randf(-1.f, 1.f);
	const auto vy = randf(-1.f, 0.f);
	p.setBaseVelocity({vx, vy});
	p.applyTimescale(timestep_scale);
}

void ParticleSystem::init_particles()
{
	for (auto &p : particles)
		init_particle(p);
}

void ParticleSystem::teleport_particle_opposite_side(Particle &p)
{
	const float radius = p.getRadius();
	p.setPosition({
		randf(rect.position.x, rect.position.x + rect.size.x),
		// Randomize the start height slightly below the view so they
		// "drift" in rather than appearing in a flat line.
		rect.position.y + rect.size.y + randf(radius, radius + 20.f),
	});
}

} // namespace avz
