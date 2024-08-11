#pragma once

#include <functional>

#include "tt/Particle.hpp"
#include "tt/StereoAnalyzer.hpp"
#include "tt/MonoAnalyzer.hpp"
#include <SFML/Graphics.hpp>

namespace viz
{

class ParticleSystem : public sf::Drawable
{
public:
	struct UpdateOptions
	{
		float calm_factor;
		std::function<float(float)> weight_func, displacement_func;
	};

private:
	inline static const UpdateOptions default_update_opts = {5, sqrtf, sqrtf};

	sf::IntRect rect;
	std::vector<tt::Particle<sf::CircleShape>> particles;

public:
	ParticleSystem(const sf::IntRect &rect, unsigned particle_count = 30);
	void update(sf::Vector2f additional_displacement = {0, 0});
	void update(const tt::MonoAnalyzer &ma, float scale_factor, const UpdateOptions &options = default_update_opts);
	void update(const tt::StereoAnalyzer &sa, float scale_factor, const UpdateOptions &options = default_update_opts);
	void draw(sf::RenderTarget &, sf::RenderStates) const;
};

} // namespace viz
