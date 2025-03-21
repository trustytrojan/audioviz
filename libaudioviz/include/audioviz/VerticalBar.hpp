#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz
{

/**
 * An `sf::RectangleShape` whose origin is at the bottom-left corner of the rectangle.
 * Implements `setWidth(float)` and `setHeight(float)` for use with `viz::SpectrumDrawable`.
 */
class VerticalBar : public sf::RectangleShape
{
public:
	VerticalBar(float width = 0, float height = 0);
	void setWidth(float width);
	void setHeight(float height);
};

} // namespace viz
