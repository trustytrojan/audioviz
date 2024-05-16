#pragma once

#include <SFML/Graphics.hpp>

namespace tt
{

/**
 * A `Particle` is an object that can freely move in 2D space.
 * The type of object that moves is up to the caller.
 * @tparam ShapeType Must be a subclass of `sf::Transformable`.
 */
template <typename ShapeType>
class Particle : public ShapeType
{
	static_assert(std::is_base_of_v<sf::Transformable, ShapeType>, "ShapeType must be a subclass of sf::Transformable");
	sf::Vector2f velocity;

public:
	Particle() : ShapeType() {}

	void updatePosition()
	{
		this->setPosition(this->getPosition() + velocity);
	}

	void setVelocity(const sf::Vector2f velocity)
	{
		this->velocity = velocity;
	}
};

} // namespace tt
