#pragma once

#include <audioviz/fx/Effect.hpp>

namespace audioviz::fx
{

// Adds an addend to the entire texture.
class Alpha : public Effect
{
	static inline sf::Shader shader{std::filesystem::path{"shaders/alpha.frag"}, sf::Shader::Type::Fragment};

public:
	float alpha;

	Alpha(float alpha);
	void apply(RenderTexture &rt) const override;
};

} // namespace audioviz::fx
