#include <audioviz/Layer.hpp>

namespace audioviz
{

Layer::Layer(const std::string &name, const sf::Vector2u size, const unsigned antialiasing)
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

void Layer::add_draw(DrawCall dc)
{
	draws.emplace_back(dc);
}

void Layer::set_fx_cb(const FxCb &cb)
{
	fx_cb = cb;
}

void Layer::full_lifecycle(sf::RenderTarget &target)
{
	if (!draws.empty())
		_orig_rt.clear(sf::Color::Transparent);
	for (const auto dc : draws)
	{
		sf::RenderStates rs;
		if (dc.transform_effect)
		{
			dc.transform_effect->setShaderUniforms();
			rs.shader = &dc.transform_effect->getShader();
		}
		_orig_rt.draw(dc.drawable, rs);
	}
	if (!draws.empty())
		_orig_rt.display();
	_fx_rt.clear(sf::Color::Transparent);
	_fx_rt.copy(_orig_rt);
	for (const auto effect : effects)
		effect->apply(_fx_rt); // this will call display() on _fx_rt for us
	if (fx_cb)
		fx_cb(_orig_rt, _fx_rt, target);
}

} // namespace audioviz
