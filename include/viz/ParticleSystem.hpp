#pragma once

#include "tt/Particle.hpp"
#include <SFML/Graphics.hpp>

namespace viz
{

class ParticleSystem : public sf::Drawable
{
	sf::Vector2u target_size;
	std::vector<tt::Particle<sf::CircleShape>> particles;
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

} // namespace viz
