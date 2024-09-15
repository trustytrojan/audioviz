#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <random>

#include "tt/AudioAnalyzer.hpp"
#include "tt/Particle.hpp"
#include "viz/util.hpp"

namespace viz
{

template <typename ParticleShape>
class ParticleSystem : public sf::Drawable
{
public:
	struct UpdateOptions
	{
		float calm_factor = 5, multiplier = 1;
		std::function<float(float)> weight_func = sqrtf, displacement_func = sqrtf;
	};

private:
	template <typename T>
	static T random(const T min, const T max)
	{
		static std::mt19937 gen(std::random_device{}());
		return std::uniform_real_distribution<>(min, max)(gen);
	}

	sf::IntRect rect;
	std::vector<tt::Particle<ParticleShape>> particles;

public:
	ParticleSystem(const sf::IntRect &rect, unsigned particle_count)
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

	void update(sf::Vector2f additional_displacement = {0, 0})
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

	// uhhh.. this is bad. making `aa` pass-by-reference causes the particles to never appear on screen...
	// figure this out!!!!!!!!!!!!!!!!!!
	void update(const tt::AudioAnalyzer aa, const UpdateOptions &options = {})
	{
		float avg;
		for (int i = 0; i < aa.get_num_channels(); ++i)
			avg += util::weighted_max(aa.get_spectrum_data(i), options.weight_func);
		avg /= aa.get_num_channels();
		const auto scaled_avg = rect.size.y * avg;
		const auto additional_displacement = options.displacement_func(scaled_avg / options.calm_factor);
		update({0, -additional_displacement * options.multiplier});
	}

	void draw(sf::RenderTarget &target, sf::RenderStates) const override
	{
		for (const auto &p : particles)
			target.draw(p);
	}
};

} // namespace viz
