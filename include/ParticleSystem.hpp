#pragma once

#include <SFML/Graphics.hpp>

class Particle : public sf::CircleShape
{
	sf::Vector2f velocity;

public:
	Particle(float radius = 0, std::size_t pointCount = 15)
		: sf::CircleShape(radius, pointCount) {}

	void move()
	{
		const auto [x, y] = getPosition();
		setPosition({x + velocity.x, y + velocity.y});
	}

	sf::Vector2f getVelocity() const
	{
		return velocity;
	}

	void setVelocity(sf::Vector2f velocity)
	{
		this->velocity = velocity;
	}
};

class ParticleSystem
{
	sf::Vector2u target_size;
	std::vector<Particle> particles;
	unsigned max_height;

	// debugging constructs
	bool debug;
	sf::VertexArray max_height_line = sf::VertexArray(sf::PrimitiveType::Lines, 2);
	int particle_to_trace_idx;

public:
	ParticleSystem(sf::Vector2u target_size, unsigned particle_count = 30, bool debug = false);
	void set_max_height(unsigned max_height);
	void draw(sf::RenderTarget &target, sf::Vector2f additional_displacement = {0, 0});
};