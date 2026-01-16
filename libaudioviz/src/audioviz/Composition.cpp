#include <algorithm>
#include <audioviz/Composition.hpp>

namespace audioviz
{

Composition::Composition(const sf::Vector2u size)
	: final_rt{size}
{
}

void Composition::compose()
{
	final_rt.clear();
	for (auto &layer : layers)
		layer.full_lifecycle(final_rt);
	final_rt.display();
}

void Composition::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	target.draw(final_rt.sprite());
}

Layer &Composition::add_layer(const std::string &name, const int antialiasing)
{
	return layers.emplace_back(name, final_rt.getSize(), antialiasing);
}

Layer *Composition::get_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](auto &l) { return l.get_name() == name; });
	return (itr == layers.end()) ? nullptr : itr.base();
}

void Composition::remove_layer(const std::string &name)
{
	const auto &itr = std::ranges::find_if(layers, [&](auto &l) { return l.get_name() == name; });
	if (itr != layers.end())
		layers.erase(itr);
}

} // namespace audioviz
