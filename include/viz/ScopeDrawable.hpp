#pragma once

#include "tt/ColorUtils.hpp"

#include "SpectrumBar.hpp"

namespace viz
{

template <typename ShapeType>
class ScopeDrawable
{
	static_assert(std::is_base_of_v<sf::Transformable, ShapeType>, "ShapeType must be a subclass of sf::Transformable");

	// internal data
	std::vector<ShapeType> shapes;
	sf::IntRect rect;
	bool backwards = false;
	int spacing = 5;

public:


private:
	void update_shapes()
	{
		const int bar_count = rect.width / (bar.width + bar.spacing);
	}
};

} // namespace viz
