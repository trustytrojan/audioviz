#include <audioviz/VerticalBar.hpp>

namespace audioviz
{

VerticalBar::VerticalBar(float width, float height)
	: sf::RectangleShape({width, height})
{
}

void VerticalBar::setWidth(float width)
{
	setSize({width, getSize().y});
	update();
}

void VerticalBar::setHeight(float height)
{
	setSize({getSize().x, height});
	setOrigin({0, height});
	update();
}

} // namespace audioviz
