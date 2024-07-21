#pragma once

#include <SFML/Graphics.hpp>

namespace viz
{

template <typename T>
class SpectrumBar : public T
{
	static_assert(std::is_base_of_v<sf::Shape, T>, "T must be a subclass of sf::Shape");

public:
	template <typename... Args>
	SpectrumBar(Args &&...args) : T(std::forward<Args>(args)...) {}

	virtual void setWidth(const float width) = 0;
	virtual void setHeight(const float height) = 0;
};

} // namespace viz
