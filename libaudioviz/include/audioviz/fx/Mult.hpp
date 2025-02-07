#pragma once

#include <audioviz/fx/Effect.hpp>

namespace audioviz::fx
{

// Multiplies the entire texture by a factor.
class Mult : public Effect
{
	static inline sf::Shader shader{std::filesystem::path{"shaders/mult.frag"}, sf::Shader::Type::Fragment};

public:
	float factor;

	Mult(float factor);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
