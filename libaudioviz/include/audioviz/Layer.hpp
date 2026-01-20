#pragma once

#include "audioviz/fx/TransformEffect.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

namespace audioviz
{

struct DrawCall
{
	const sf::Drawable &drawable;
	const fx::TransformEffect *const transform_effect{};

	// this isn't needed anymore, but maybe it will be in the future
	// const sf::RenderStates states;
};

class Layer
{
	std::string name;
	std::vector<DrawCall> draws;

public:
	inline Layer(const std::string &name)
		: name{name}
	{
	}

	inline const std::string &get_name() const { return name; }
	inline void add_draw(DrawCall dc) { draws.emplace_back(dc); }

	virtual void render(sf::RenderTarget &target);
};

} // namespace audioviz
