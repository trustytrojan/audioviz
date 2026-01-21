#include <algorithm>
#include "Base.hpp"

namespace avz
{

Base::Base(const sf::Vector2u size)
	: size{size},
	  final_rt{size}
{
	profiler_text.setPosition({size.x - size.x * (500.f / 1280.f), size.y * (30.f / 720.f)});
	profiler_text.setCharacterSize(size.y * (18.f / 720.f));
	profiler_text.setFillColor({255, 255, 255, 150});
}

void Base::next_frame(const std::span<const float> audio_buffer)
{
	capture_time("update", update(audio_buffer));

	final_rt.clear();
	for (const auto &layer : layers)
		capture_time("layer '" + layer->get_name() + '\'', layer->render(final_rt));
	final_rt.display();

	if (profiler_enabled)
		profiler_text.setString(profiler.getSummary());
}

void Base::draw(sf::RenderTarget &target, sf::RenderStates) const
{
	target.draw(final_rt);
	if (profiler_enabled)
		target.draw(profiler_text);
}

Layer *Base::get_layer(const std::string &name)
{
	auto itr = std::ranges::find_if(layers, [&](auto &l) { return l->get_name() == name; });
	return (itr == layers.end()) ? nullptr : itr->get();
}

} // namespace avz
