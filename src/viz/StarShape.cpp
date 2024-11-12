#include "viz/StarShape.hpp"
#include <cmath>

namespace viz
{

StarShape::StarShape(const float outerR, const float innerR)
	: sf::ConvexShape(10)
{
}

void StarShape::setRadii(const float outerR, const float innerR)
{
	static const float radianvalues[] = {
		0.314159, 0.942478, 1.5708, 2.19911, 2.82743, 3.45575, 4.08407, 4.71239, 5.34071, 5.96903};
	for (int i = 0; i < 9; i += 2)
	{
		setPoint(i, {outerR * cos(radianvalues[i]), outerR * sin(radianvalues[i])});
		setPoint(i + 1, {innerR * cos(radianvalues[i + 1]), innerR * sin(radianvalues[i + 1])});
	}
}

} // namespace viz
