#include <avz/Layer.hpp>

namespace audioviz
{

void Layer::render(sf::RenderTarget &target)
{
	for (const auto dc : draws)
	{
		sf::RenderStates rs;
		if (dc.transform_effect)
		{
			dc.transform_effect->setShaderUniforms();
			rs.shader = &dc.transform_effect->getShader();
		}
		target.draw(dc.drawable, rs);
	}
}

} // namespace audioviz
