#pragma once

#include <audioviz/fx/Effect.hpp>

namespace audioviz::fx
{

// Adds an addend to the entire texture.
class Add : public Effect
{
	static inline sf::Shader shader{std::filesystem::path{"shaders/add.frag"}, sf::Shader::Type::Fragment};

public:
	float addend;

	Add(float addend);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
