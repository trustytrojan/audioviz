#pragma once

#include "fx/Effect.hpp"
#include "tt/RenderTexture.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>

namespace viz
{

class Layer
{
public:
	using OrigCb = std::function<void(tt::RenderTexture &)>;
	using FxCb = std::function<void(const tt::RenderTexture &, const tt::RenderTexture &, sf::RenderTarget &)>;

	std::string name;

private:
	tt::RenderTexture _orig_rt, _fx_rt;
	bool auto_fx = true;
	OrigCb orig_cb;
	FxCb fx_cb;

public:
	/**
	 * The effects, in order, that will be applied to the "original"
	 * render-texture when `apply_fx()` is called.
	 */
	std::vector<std::unique_ptr<fx::Effect>> effects;

	Layer(const std::string &name, sf::Vector2u size, int antialiasing);

	/**
	 * @brief Clear the "original" render-texture with `color`.
	 */
	void orig_clear(sf::Color color = sf::Color::Transparent);

	/**
	 * @brief Draw `drawable` onto the "original" render-texture.
	 *        Immediately calls `display` on it as well.
	 */
	void orig_draw(const sf::Drawable &drawable);

	void orig_display();

	void set_orig_cb(const OrigCb &cb);
	void set_auto_fx(const bool b);
	void set_fx_cb(const FxCb &cb);

	/**
	 * @brief Copies the "original" render-texture to the "effects" render-texture
	 *        and applies all `effects` on it.
	 */
	void apply_fx();

	void full_lifecycle(sf::RenderTarget &target);

private:
};

} // namespace viz
