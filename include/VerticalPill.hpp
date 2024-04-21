#pragma once

#include <SFML/Graphics.hpp>

class VerticalPill : public sf::CircleShape
{
	// width of the pill, which is always double the radius
	float width;
	
	// height of the "rectangle" between the two semicircles
	float height;

public:
	VerticalPill(float width = 0, float height = 0, std::size_t pointCount = 30);
	void setWidth(float width);
	void setHeight(float height);
	sf::Vector2f getPoint(std::size_t index) const override;
};