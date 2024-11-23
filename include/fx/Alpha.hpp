#pragma once

#include "Effect.hpp"

namespace fx
{

// Adds an addend to the entire texture.
class Alpha : public Effect
{
	static inline sf::Shader shader = sf::Shader(std::filesystem::path{"shaders/alpha.frag"}, sf::Shader::Type::Fragment);

public:
	float alpha;

	Alpha(float alpha);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
