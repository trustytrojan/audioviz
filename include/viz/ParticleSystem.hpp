#pragma once

#include "tt/Particle.hpp"
#include "tt/StereoAnalyzer.hpp"
#include <SFML/Graphics.hpp>

namespace viz
{

class ParticleSystem : public sf::Drawable
{
	sf::IntRect rect;
	std::vector<tt::Particle<sf::CircleShape>> particles;

public:
	ParticleSystem(const sf::IntRect &rect, unsigned particle_count = 30);
	void update(sf::Vector2f additional_displacement = {0, 0});
	void update(const tt::StereoAnalyzer &sa, float scale);
	void draw(sf::RenderTarget &, sf::RenderStates) const;
};

} // namespace viz
