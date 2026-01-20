#pragma once

#include <SFML/Graphics.hpp>

namespace audioviz::fx
{

class TransformEffect
{
public:
	virtual ~TransformEffect() = default;

	// Get the shader object associated with this transform effect.
	virtual const sf::Shader &getShader() const = 0;

	// Sets this transform effect's parameters as the shader object's uniforms.
	virtual void setShaderUniforms() const = 0;
};

} // namespace audioviz::fx
