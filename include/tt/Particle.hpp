#pragma once

#include <SFML/Graphics.hpp>

namespace tt {

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
