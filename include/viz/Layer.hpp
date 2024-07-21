#pragma once

#include "fx/Effect.hpp"
#include "tt/RenderTexture.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

namespace viz
{

class Layer
{
	tt::RenderTexture _orig_rt, _fx_rt;

public:
	std::vector<std::unique_ptr<fx::Effect>> effects;

	Layer(sf::Vector2u size, int antialiasing);

	/**
	 * @brief Clear the "original" render-texture with `color`.
	 */
	void orig_clear(sf::Color color = sf::Color::Transparent);

	/**
	 * @brief Draw `drawable` onto the "original" render-texture.
	 *        Immediately calls `display` on it as well.
	 */
	void orig_draw(const sf::Drawable &drawable);

	/**
	 * @brief Copies the "original" render-texture to the "effects" render-texture
	 *        and applies all `effects` on it.
	 */
	void apply_fx();

	/**
	 * @return The "original" render-texture. Contains what was drawn to it using
	 *         `orig_clear` or `orig_draw`.
	 */
	const tt::RenderTexture &orig_rt() const { return _orig_rt; }

	/**
	 * @return The "effects" render-texture. Contains the result of the applied effects
	 *         on the "original" render-texture (calling `apply_fx`).
	 */
	const tt::RenderTexture &fx_rt() const { return _fx_rt; }
};

} // namespace viz
