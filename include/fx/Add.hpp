#pragma once

#include "Effect.hpp"

namespace fx
{

// Adds an addend to the entire texture.
class Add : public Effect
{
	static inline sf::Shader shader = sf::Shader(std::filesystem::path{"shaders/add.frag"}, sf::Shader::Type::Fragment);

public:
	float addend;

	Add(float addend);
	void apply(tt::RenderTexture &rt) const override;
};

} // namespace fx
