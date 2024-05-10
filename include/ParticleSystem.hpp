#pragma once

#include <SFML/Graphics.hpp>

class Particle : public sf::CircleShape
{
	sf::Vector2f velocity;

public:
	Particle() : sf::CircleShape(0, 15) {}

	void move()
	{
		setPosition(getPosition() + velocity);
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

class ParticleSystem : public sf::Drawable
{
	sf::Vector2u target_size;
	std::vector<Particle> particles;
	unsigned max_height = 0;

	// debugging constructs
	bool debug;
	sf::VertexArray max_height_line = sf::VertexArray(sf::PrimitiveType::Lines, 2);
	int particle_to_trace_idx;

public:
	ParticleSystem(sf::Vector2u target_size, unsigned particle_count = 30, bool debug = false);
	void set_max_height(unsigned max_height);
	void update(sf::Vector2f additional_displacement = {0, 0});
	void draw(sf::RenderTarget &, sf::RenderStates) const;
};