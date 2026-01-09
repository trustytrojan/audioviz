#include <audioviz/Layer.hpp>

namespace audioviz
{

Layer::Layer(const std::string &name, const sf::Vector2u size, const int antialiasing)
	: name{name},
	  _orig_rt{size, antialiasing},
	  _fx_rt{size}
{
}

void Layer::add_effect(fx::PostProcessEffect *const effect)
{
	effect->setRtSize(_fx_rt.getSize());
	effects.emplace_back(effect);
}

void Layer::add_drawable(const sf::Drawable *const drawable)
{
	drawables.emplace_back(drawable);
}

void Layer::orig_draw(const sf::Drawable &drawable)
{
	_orig_rt.draw(drawable);
}

void Layer::orig_display()
{
	_orig_rt.display();
}

void Layer::set_orig_cb(const OrigCb &cb)
{
	orig_cb = cb;
}

void Layer::set_auto_fx(const bool b)
{
	auto_fx = b;
}

void Layer::set_fx_cb(const FxCb &cb)
{
	fx_cb = cb;
}

void Layer::apply_fx()
{
	_fx_rt.clear(sf::Color::Transparent);
	_fx_rt.copy(_orig_rt);
	for (const auto effect : effects)
		effect->apply(_fx_rt); // this will call display() on _fx_rt for us
}

void Layer::full_lifecycle(sf::RenderTarget &target)
{
	if (!drawables.empty())
		_orig_rt.clear(sf::Color::Transparent);
	if (orig_cb)
		orig_cb(_orig_rt);
	for (const auto drawable : drawables)
		_orig_rt.draw(*drawable);
	if (!drawables.empty())
		_orig_rt.display();
	if (auto_fx)
		apply_fx();
	if (fx_cb)
		fx_cb(_orig_rt, _fx_rt, target);
}

} // namespace audioviz
