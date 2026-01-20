#pragma once

#include "avz/fft/MultiChannelAudioAnalyzer.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <random>

#include <avz/Particle.hpp>
#include <avz/fft/AudioAnalyzer.hpp>
#include <avz/util.hpp>

namespace audioviz
{

template <typename ParticleShape>
class ParticleSystem : public sf::Drawable
{
public:
	struct UpdateOptions
	{
		float calm_factor{5}, multiplier{1};
		std::function<float(float)> weight_func{sqrtf}, displacement_func{sqrtf};
	};

private:
	template <typename T>
	static T random(const T min, const T max)
	{
		static std::mt19937 gen(std::random_device{}());
		return std::uniform_real_distribution<>(min, max)(gen);
	}

	sf::IntRect rect;
	std::vector<Particle<ParticleShape>> particles;
	float timestep_scale{1.f};
	bool debug_rect{};

public:
	ParticleSystem(const sf::IntRect &rect, const int particle_count)
		: rect{rect},
		  particles{particle_count}
	{
		init_particles();
	}

	ParticleSystem(const sf::IntRect &rect, const int particle_count, const int framerate)
		: rect{rect},
		  particles{particle_count},
		  timestep_scale{framerate > 0 ? 60.f / framerate : 1.f}
	{
		init_particles();
	}

	void set_framerate(int framerate)
	{
		timestep_scale = framerate > 0 ? 60.f / framerate : 1.f;
		for (auto &p : particles)
			update_particle_velocity(p);
	}

	void update(float additional_displacement = 0.f)
	{
		const float scaled_displacement = additional_displacement * timestep_scale;
		const sf::Vector2f displacement{0.f, -scaled_displacement};

		for (auto &p : particles)
		{
			// let the particle move using its velocity
			p.updatePosition();

			// then apply additional displacement upward
			auto new_pos = p.getPosition() + displacement;

			float width = 0;
			if constexpr (std::is_same_v<ParticleShape, sf::CircleShape>)
				width = p.getRadius();
			else
				width = p.getSize().x;

			// make sure particles don't escape the rect
			if (new_pos.x >= (rect.position.x + rect.size.x))
				// teleport from right edge to left
				new_pos.x = rect.position.x + -width;
			else if (new_pos.x + width < 0)
				// teleport from left edge to right
				new_pos.x = (rect.position.x + rect.size.x);

			p.setPosition(new_pos);

			// sqrt of remaining distance to 0 causes the fading out to only start halfway up the screen
			// linear is too sudden

			// decrease alpha with distance from bottom
			auto [r, g, b, a] = p.getFillColor();
			a = sqrtf((new_pos.y - rect.position.y) / rect.size.y) * 255;
			p.setFillColor({r, g, b, a});

			// ONLY reset position to start side once it reaches opposite side
			if (new_pos.y <= rect.position.y)
				p.setPosition(
					{random<float>(rect.position.x, rect.position.x + rect.size.x), rect.position.y + rect.size.y});
		}
	}

	void update(MultiChannelAudioAnalyzer &ma, const UpdateOptions &options = {})
	{
		const auto scaled_avg = rect.size.y * ma.compute_averaged_peak_frequency(0, 250).amplitude;
		const auto additional_displacement = options.displacement_func(scaled_avg / options.calm_factor);
		update(additional_displacement * options.multiplier);
	}

	void update(AudioAnalyzer &aa, const UpdateOptions &options = {})
	{
		const auto scaled_avg = rect.size.y * aa.compute_peak_frequency(0, 250).amplitude;
		const auto additional_displacement = options.displacement_func(scaled_avg / options.calm_factor);
		update(additional_displacement * options.multiplier);
	}

	inline void set_debug_rect(bool b) { debug_rect = b; }

	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override
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

	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;

		// Check each particle and reinitialize only those out of bounds
		for (auto &p : particles)
			if (!rect.contains((sf::Vector2i)p.getPosition()))
				teleport_particle_opposite_side(p);
	}

	void set_particle_count(const size_t count)
	{
		particles.resize(count);
		init_particles();
	}

	void set_particle_textures(const sf::Texture &txr)
	{
		for (auto &p : particles)
			p.setTexture(&txr);
	}

	void set_color(const sf::Color color)
	{
		for (auto &p : particles)
			p.setFillColor(color);
	}

	inline size_t get_particle_count() const { return particles.size(); }
	inline sf::IntRect get_rect() const { return rect; }
	inline bool get_debug_rect() const { return debug_rect; }

private:
	void init_particle(Particle<ParticleShape> &p)
	{
		if constexpr (std::is_same_v<ParticleShape, sf::CircleShape>)
		{
			// these are going to be very small circles, don't need many points
			p.setPointCount(10);
			p.setRadius(random<float>(2, 5));
		}
		else if constexpr (std::is_same_v<ParticleShape, sf::RectangleShape>)
		{
			p.setSize({random<float>(2, 5), random<float>(2, 5)});
		}

		// start some particles offscreen, so the beginning sequence feels less "sudden"
		// otherwise all of them come out at once and it looks bad
		p.setPosition({
			random<float>(rect.position.x, rect.position.x + rect.size.x),
			rect.position.y + rect.size.y * random<float>(1, 2),
		});

		const auto vx = random<float>(-0.5f, 0.5f) * timestep_scale;
		const auto vy = random<float>(-2.f, 0.f) * timestep_scale;
		p.setVelocity({vx, vy});
	}

	void update_particle_velocity(Particle<ParticleShape> &p)
	{
		const auto [x, y] = p.getVelocity();
		p.setVelocity({x * timestep_scale, y * timestep_scale});
	}

	void init_particles()
	{
		for (auto &p : particles)
			init_particle(p);
	}

	void teleport_particle_opposite_side(Particle<ParticleShape> &p)
	{
		p.setPosition({random<float>(rect.position.x, rect.position.x + rect.size.x), rect.position.y + rect.size.y});
	}
};

} // namespace audioviz
