#include "VerticalPill.hpp"

VerticalPill::VerticalPill(const float width, const float height, const std::size_t pointCount)
	: sf::CircleShape(width / 2, pointCount), width(width), height(height)
{
	setOrigin({0, height + width});
	update();
}

void VerticalPill::setWidth(const float width)
{
	setRadius(width / 2);
	update();
}

void VerticalPill::setHeight(const float height)
{
	this->height = height;
	setOrigin({0, height + 2 * getRadius()});
	update();
}

sf::Vector2f VerticalPill::getPoint(const std::size_t index) const
{
	const auto angle = static_cast<float>(index) / static_cast<float>(getPointCount()) * sf::degrees(360);
	auto point = sf::Vector2f(getRadius(), getRadius()) + sf::Vector2f(getRadius(), angle);

	if (index <= getPointCount() / 2)
		point.y += height;

	// need to move the edges of the top semicircle
	// to fit with the bottom semicircle
	// this is because we don't have 2 extra points to form the center "rectangle"
	if (index == 0)
		point.x -= 0.5;
	else if (index == getPointCount() / 2)
		point.x += 0.5;

	return point;
}