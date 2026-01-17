#include <audioviz/Layer.hpp>

namespace audioviz
{

PostProcessLayer::PostProcessLayer(const std::string &name, const sf::Vector2u size, const unsigned antialiasing)
	: Layer{name},
	  _orig_rt{size, antialiasing},
	  _fx_rt{size}
{
}

void PostProcessLayer::add_effect(fx::PostProcessEffect *const effect)
{
	effect->setRtSize(_fx_rt.getSize());
	effects.emplace_back(effect);
}

void PostProcessLayer::render(sf::RenderTarget &target)
{
	// reuse superclass's code to draw all the drawables onto _orig_rt
	_orig_rt.clear(sf::Color::Transparent);
	Layer::render(_orig_rt);
	_orig_rt.display();

	// copy to fx_rt, apply effects
	_fx_rt.clear(sf::Color::Transparent);
	_fx_rt.copy(_orig_rt);
	for (const auto effect : effects)
		effect->apply(_fx_rt);

	// execute custom logic for compositing the textures onto the target
	if (fx_cb)
		fx_cb(_orig_rt, _fx_rt, target);
}

} // namespace audioviz
