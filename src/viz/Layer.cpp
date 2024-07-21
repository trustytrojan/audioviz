#include "viz/Layer.hpp"

namespace viz
{

Layer::Layer(const sf::Vector2u size, const int antialiasing)
	: _orig_rt(size, antialiasing),
	  _fx_rt(size, 0) {}

void Layer::orig_clear(const sf::Color color)
{
	_orig_rt.clear(color);
}

void Layer::orig_draw(const sf::Drawable &drawable)
{
	_orig_rt.draw(drawable);
	_orig_rt.display();
}

void Layer::apply_fx()
{
	_fx_rt.clear(sf::Color::Transparent);
	_fx_rt.copy(_orig_rt);
	for (const auto &effect : effects)
		effect->apply(_fx_rt);
}

} // namespace viz
