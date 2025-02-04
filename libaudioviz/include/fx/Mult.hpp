#pragma once

#include "Effect.hpp"

namespace fx
{

// Multiplies the entire texture by a factor.
class Mult : public Effect
{
	static inline sf::Shader shader = sf::Shader(std::filesystem::path{"shaders/mult.frag"}, sf::Shader::Type::Fragment);

public:
	float factor;

	Mult(float factor);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
