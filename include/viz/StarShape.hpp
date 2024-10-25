#pragma once
#include <SFML/Graphics.hpp>

namespace viz
{

class StarShape : public sf::ConvexShape
{
public:
	StarShape(float outerR = 0, float innerR = 0);
    void setRadii(float outerR, float innerR);
};

} // namespace viz