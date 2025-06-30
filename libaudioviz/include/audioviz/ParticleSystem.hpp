#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <random>

#include <audioviz/Particle.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/util.hpp>

namespace audioviz
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
		BOTTOM,
		TOP,
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
	std::vector<Particle<ParticleShape>> particles;
	sf::Vector2f displacement_direction{0, 1};
	StartSide start_side = StartSide::BOTTOM;
	bool debug_rect{};

public:
	ParticleSystem(const sf::IntRect &rect, const int particle_count)
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

			float width = 0;
			if constexpr (std::is_same_v<ParticleShape, sf::CircleShape>)
				width = p.getRadius();
			else
				width = p.getSize().x;

			// make sure particles don't escape the rect
			switch (start_side)
			{
			case StartSide::BOTTOM:
			case StartSide::TOP:
				if (new_pos.x >= (rect.position.x + rect.size.x))
					// teleport from right edge to left
					new_pos.x = rect.position.x + -width;
				else if (new_pos.x + width < 0)
					// teleport from left edge to right
					new_pos.x = (rect.position.x + rect.size.x);
				break;
			case StartSide::LEFT:
			case StartSide::RIGHT:
				if (new_pos.y >= (rect.position.y + rect.size.y))
					// teleport from bottom edge to top
					new_pos.y = rect.position.y + -width;
				else if (new_pos.y + width < 0)
					// teleport from top edge to bottom
					new_pos.y = (rect.position.y + rect.size.y);
				break;
			}

			p.setPosition(new_pos);

			// sqrt of remaining distance to 0 causes the fading out to only start halfway up the screen
			// linear is too sudden

			// decrease alpha with distance from start side
			auto [r, g, b, a] = p.getFillColor();
			switch (start_side)
			{
			case StartSide::BOTTOM:
				a = sqrtf((new_pos.y - rect.position.y) / rect.size.y) * 255;
				break;
			case StartSide::TOP:
				a = sqrtf((rect.size.y - new_pos.y) / rect.size.y) * 255;
				break;
			case StartSide::LEFT:
				a = sqrtf((rect.size.x - new_pos.x) / rect.size.x) * 255;
				break;
			case StartSide::RIGHT:
				a = sqrtf((new_pos.x - rect.position.x) / rect.size.x) * 255;
				break;
			}
			p.setFillColor({r, g, b, a});

			// reset position to start side once it reaches opposite side
			switch (start_side)
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
					p.setPosition({
						rect.position.x + rect.size.x,
						random<float>(rect.position.y, rect.position.y + rect.size.y),
					});
				break;
			}
		}
	}

	void update(const std::vector<float> &spectrum_data, const UpdateOptions &options = {})
	{
		const auto scaled_avg = rect.size.y * util::weighted_max(spectrum_data, options.weight_func);
		const auto additional_displacement = options.displacement_func(scaled_avg / options.calm_factor);
		update(displacement_direction * additional_displacement * options.multiplier);
	}

	void update(const fft::AudioAnalyzer &aa, const UpdateOptions &options = {})
	{
		float avg{}; // didn't initialize this for the longest time... yikes.
		for (int i = 0; i < aa.get_num_channels(); ++i)
			avg += util::weighted_max(aa.get_spectrum_data(i), options.weight_func);
		avg /= aa.get_num_channels();
		const auto scaled_avg = rect.size.y * avg;
		const auto additional_displacement = options.displacement_func(scaled_avg / options.calm_factor);
		update(displacement_direction * additional_displacement * options.multiplier);
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

	void set_displacement_direction(sf::Vector2f displacement) { displacement_direction = displacement; }

	void set_start_side(StartSide side)
	{
		start_side = side;
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

private:
	void init_particles()
	{
		for (auto &p : particles)
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
			// sets displacement_direction based on which side we are starting on
			switch (start_side)
			{
			case StartSide::BOTTOM:
				p.setPosition({
					random<float>(rect.position.x, rect.position.x + rect.size.x),
					(rect.position.y + rect.size.y) * random<float>(1, 1.5),
				});
				p.setVelocity({random<float>(-0.5, 0.5), random<float>(-2, 0)});
				displacement_direction = {0, -1};
				break;
			case StartSide::TOP:
				p.setPosition({
					random<float>(rect.position.x, rect.position.x + rect.size.x),
					(rect.position.y + rect.size.y) * random<float>(-.5, 0),
				});
				p.setVelocity({random<float>(-0.5, 0.5), random<float>(0, 2)});
				displacement_direction = {0, 1};
				break;
			case StartSide::LEFT:
				p.setPosition({
					(rect.position.x + rect.size.x) * random<float>(-1, 0),
					random<float>(rect.position.y, rect.position.y + rect.size.y),
				});
				p.setVelocity({random<float>(0, 2), random<float>(-0.5, 0.5)});
				displacement_direction = {1, 0};
				break;
			case StartSide::RIGHT:
				p.setPosition({
					(rect.position.x + rect.size.x) * random<float>(1, 2),
					random<float>(rect.position.y, rect.position.y + rect.size.y),
				});
				p.setVelocity({random<float>(-2, 0), random<float>(-0.5, 0.5)});
				displacement_direction = {-1, 0};
				break;
			}
		}
	}
};

} // namespace audioviz
