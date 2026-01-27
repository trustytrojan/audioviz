#pragma once

#include <SFML/Graphics.hpp>

namespace avz
{

class ParticleSystem : public sf::Drawable
{
private:
	// leave private here until there is another usecase
	class Particle : public sf::CircleShape
	{
		sf::Vector2f velocity;

	public:
		inline void updatePosition() { setPosition(getPosition() + velocity); }
		inline void setVelocity(const sf::Vector2f v) { velocity = v; }
		inline sf::Vector2f getVelocity() const { return velocity; }
		inline void scaleVelocity(const float x) { velocity *= x; }
	};

	sf::IntRect rect;
	std::vector<Particle> particles;
	float timestep_scale{1.f};
	bool debug_rect{};
	bool fade_out{true};
	bool start_offscreen{true};

public:
	inline ParticleSystem(const sf::IntRect &rect, const int particle_count)
		: rect{rect},
		  particles{particle_count}
	{
		init_particles();
	}

	inline ParticleSystem(const sf::IntRect &rect, const int particle_count, const int framerate)
		: rect{rect},
		  particles{particle_count},
		  timestep_scale{framerate > 0 ? 60.f / framerate : 1.f}
	{
		init_particles();
	}

	/**
	 * Scales the particle's velocity by `60 / framerate`, keeping particle speed
	 * consistent regardless of the caller's framerate.
	 */
	void set_framerate(int framerate);

	/**
	 * Update the system, applying an additional displacement to all particles if nonzero.
	 */
	void update(const float additional_displacement = 0.f);

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline void set_fade_out(bool b) { fade_out = b; }

	inline void set_start_offscreen(bool b)
	{
		if (start_offscreen == b)
			return;
		start_offscreen = b;
		init_particles();
	}

	void draw(sf::RenderTarget &target, const sf::RenderStates states) const override;

	void set_rect(const sf::IntRect &rect);

	inline void set_particle_count(const size_t count)
	{
		particles.resize(count);
		init_particles();
	}

	inline size_t get_particle_count() const { return particles.size(); }
	inline sf::IntRect get_rect() const { return rect; }
	inline bool get_debug_rect() const { return debug_rect; }

private:
	void init_particle(Particle &p);
	void init_particles();
	void teleport_particle_opposite_side(Particle &p);
};

} // namespace avz
