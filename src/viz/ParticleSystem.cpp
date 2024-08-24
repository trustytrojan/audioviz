#include <random>

#include "viz/ParticleSystem.hpp"
#include "viz/util.hpp"

template <typename _Tp>
static _Tp random(const _Tp min, const _Tp max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return std::uniform_real_distribution<>(min, max)(gen);
}

namespace viz
{

ParticleSystem::ParticleSystem(const sf::IntRect &rect, unsigned particle_count)
	: rect(rect),
	  particles(particle_count)
{
	for (auto &p : particles)
	{
		// these are going to be very small circles, don't need many points
		p.setPointCount(10);

		p.setRadius(random<float>(2, 5));
		p.setVelocity({random<float>(-0.5, 0.5), random<float>(0, -2)});

		// start some particles offscreen, so the beginning sequence feels less "sudden"
		// otherwise all of them come out at once and it looks bad
		p.setPosition({random<float>(rect.position.x, rect.position.x + rect.size.x), (rect.position.y + rect.size.y) * random<float>(1, 1.5)});
	}
}

void ParticleSystem::update(const sf::Vector2f additional_displacement)
{
	for (auto &p : particles)
	{
		// let the particle move using its velocity
		p.updatePosition();

		// then apply additional displacement
		auto new_pos = p.getPosition() + additional_displacement;

		// make sure particles don't escape the rect
		if (new_pos.x >= (rect.position.x + rect.size.x))
			// teleport from right edge to left
			new_pos.x = rect.position.x + -p.getRadius();
		else if (new_pos.x + p.getRadius() < 0)
			// teleport from left edge to right
			new_pos.x = (rect.position.x + rect.size.x);

		p.setPosition(new_pos);

		// sqrt of remaining distance to 0 causes the fading out to only start halfway up the screen
		// linear is too sudden
		const auto alpha_scale = sqrtf((new_pos.y - rect.position.y) / (rect.position.y + rect.size.y));

		// decrease alpha with distance to max_height
		const auto [r, g, b, _] = p.getFillColor();
		p.setFillColor({r, g, b, alpha_scale * 255});

		// reset position to bottom of target once max_height is reached
		if (new_pos.y <= rect.position.y)
			p.setPosition({random<float>(rect.position.x, rect.position.x + rect.size.x), rect.position.y + rect.size.y});
	}
}

void ParticleSystem::update(const tt::MonoAnalyzer &sa, const float scale, const UpdateOptions &opts)
{
	const auto wmax = util::weighted_max(sa.spectrum_data(), opts.weight_func);
	const auto scaled_wmax = wmax * scale;
	const auto additional_displacement = opts.displacement_func(scaled_wmax / opts.calm_factor);
	update({0, -additional_displacement});
}

void ParticleSystem::update(const tt::StereoAnalyzer &sa, const float scale, const UpdateOptions &opts)
{
	const auto &left_data = sa.left_data(),
			   &right_data = sa.right_data();
	assert(left_data.size() == right_data.size());

	const auto avg = (util::weighted_max(left_data, opts.weight_func) +
					  util::weighted_max(right_data, opts.weight_func)) /
					 2;

	// scale by window size to keep movement consistent with all window sizes
	const auto scaled_avg = scale * avg;

	// the deciding factor in particle speed increase
	const auto additional_displacement = opts.displacement_func(scaled_avg / opts.calm_factor);

	// update particle system with additional (upward) displacement
	update({0, -additional_displacement});
}

void ParticleSystem::draw(sf::RenderTarget &target, const sf::RenderStates states) const
{
	for (const auto &p : particles)
		target.draw(p, states);
}

} // namespace viz
