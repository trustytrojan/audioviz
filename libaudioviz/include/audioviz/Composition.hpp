#pragma once

#include "Layer.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace audioviz
{

/**
 * Represents a composition of layers.
 */
class Composition : public sf::Drawable
{
	std::vector<Layer> layers;
	RenderTexture final_rt;

public:
	Composition(sf::Vector2u size);

	inline sf::Vector2u get_size() const { return final_rt.getSize(); }

	Layer &add_layer(const std::string &name, int antialiasing = 0);
	Layer *get_layer(const std::string &name);
	void remove_layer(const std::string &name);

	/**
	 * Renders all layers, in order, onto this composition.
	 */
	void compose();

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
};

} // namespace audioviz
