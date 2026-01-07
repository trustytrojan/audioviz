#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz
{

/**
 * An object that can move in 2D space.
 * The type parameter `ShapeType` must be a subclass of `sf::Transformable`.
 */
template <typename ShapeType>
class Particle : public ShapeType
{
	static_assert(std::is_base_of_v<sf::Transformable, ShapeType>, "ShapeType must be a subclass of sf::Transformable");
	sf::Vector2f velocity;

public:
	void updatePosition() { this->setPosition(this->getPosition() + velocity); }
	void setVelocity(const sf::Vector2f velocity) { this->velocity = velocity; }
};

} // namespace audioviz
