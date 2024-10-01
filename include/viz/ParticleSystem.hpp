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

	enum class StartSide
	{
		TOP,
		BOTTOM,
		LEFT,
		RIGHT
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
	sf::Vector2f displacement_direction{0, 1};

	StartSide start_position = StartSide::BOTTOM;

public:
	ParticleSystem(const size_t particle_count)
		: particles{particle_count}
	{
		init_particles();
	}

	ParticleSystem(const sf::IntRect &rect, const size_t particle_count)
		: rect{rect},
		  particles{particle_count}
	{
		init_particles();
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

			switch (start_position)
			{
			case StartSide::BOTTOM:
			case StartSide::TOP:
				if (new_pos.x >= (rect.position.x + rect.size.x))
					// teleport from right edge to left
					new_pos.x = rect.position.x + -p.getRadius();
				else if (new_pos.x + p.getRadius() < 0)
					// teleport from left edge to right
					new_pos.x = (rect.position.x + rect.size.x);
				break;
			case StartSide::LEFT:
			case StartSide::RIGHT:
				if (new_pos.y >= (rect.position.y + rect.size.y))
					// teleport from right edge to left
					new_pos.y = rect.position.y + -p.getRadius();
				else if (new_pos.y + p.getRadius() < 0)
					// teleport from left edge to right
					new_pos.y = (rect.position.y + rect.size.y);
				break;
			}

			p.setPosition(new_pos);

			// sqrt of remaining distance to 0 causes the fading out to only start halfway up the screen
			// linear is too sudden

			// decrease alpha with distance to max_height
			auto [r, g, b, a] = p.getFillColor();
			switch (start_position)
			{
			case StartSide::BOTTOM:
				a = sqrtf((new_pos.y - rect.position.y) / (rect.position.y + rect.size.y)) * 255;
				break;
			case StartSide::TOP:
				a = sqrtf((rect.size.y - new_pos.y) / (rect.position.y + rect.size.y)) * 255;
				break;
			case StartSide::LEFT:
				a = sqrtf((rect.size.x - new_pos.x) / (rect.position.x + rect.size.x)) * 255;
				break;
			case StartSide::RIGHT:
				a = sqrtf((new_pos.x - rect.position.x) / (rect.position.x + rect.size.x)) * 255;
				break;
			}
			p.setFillColor({r, g, b, a});

			// reset position to bottom of target once max_height is reached
			switch (start_position)
			{
			case StartSide::BOTTOM:
				if (new_pos.y <= rect.position.y)
					p.setPosition(
						{random<float>(rect.position.x, rect.position.x + rect.size.x), rect.position.y + rect.size.y});
				break;
			case StartSide::TOP:
				if (new_pos.y >= rect.size.x)
					p.setPosition({random<float>(rect.position.x, rect.position.x + rect.size.x), 0});
				break;
			case StartSide::LEFT:
				if (new_pos.x >= rect.size.x)
					p.setPosition({0, random<float>(rect.position.y, rect.position.y + rect.size.y)});
				break;
			case StartSide::RIGHT:
				if (new_pos.x <= rect.position.x)
					p.setPosition(
						{rect.position.x + rect.size.x, random<float>(rect.position.y, rect.position.y + rect.size.y)});
				break;
			}
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
		update(displacement_direction * additional_displacement * options.multiplier);
	}

	void draw(sf::RenderTarget &target, sf::RenderStates) const override
	{
		for (const auto &p : particles)
			target.draw(p);
	}

	void set_displacement_direction(sf::Vector2f displacement) { displacement_direction = displacement; }

	void set_start_position(StartSide side)
	{
		start_position = side;
		init_particles();
	}

	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		init_particles();
	}

	void set_particle_count(const size_t count)
	{
		particles.resize(count);
		init_particles();
	}

private:
	void init_particles()
	{
		for (auto &p : particles)
		{
			// these are going to be very small circles, don't need many points
			p.setPointCount(10);
			p.setRadius(random<float>(2, 5));

			// start some particles offscreen, so the beginning sequence feels less "sudden"
			// otherwise all of them come out at once and it looks bad
			switch (start_position)
			{
			case StartSide::BOTTOM:
				p.setPosition({
					random<float>(rect.position.x, rect.position.x + rect.size.x),
					(rect.position.y + rect.size.y) * random<float>(1, 1.5),
				});
				p.setVelocity({random<float>(-0.5, 0.5), random<float>(0, -2)});
				displacement_direction = {0, -1};
				break;
			case StartSide::TOP:
				p.setPosition({random<float>(rect.position.x, rect.position.x + rect.size.x), random<float>(-1.5, 0)});
				p.setVelocity({random<float>(-0.5, 0.5), random<float>(0, 2)});
				displacement_direction = {0, 1};
				break;
			case StartSide::RIGHT:
				p.setPosition({
					(rect.position.x + rect.size.x) * random<float>(1, 1.5),
					random<float>(rect.position.y, rect.position.y + rect.size.y),
				});
				p.setVelocity({random<float>(0, -2), random<float>(-0.5, 0.5)});
				displacement_direction = {-1, 0};
				break;
			case StartSide::LEFT:
				p.setPosition({random<float>(-1.5, 0), random<float>(rect.position.y, rect.position.y + rect.size.y)});
				p.setVelocity({random<float>(0, 2), random<float>(-0.5, 0.5)});
				displacement_direction = {1, 0};
				break;
			}
		}
	}
};

} // namespace viz
