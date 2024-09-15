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
	/**
	 * "Original" texture callback type. Supplies a reference to `_orig_rt` to the callback.
	 * This is so the caller can customize what is drawn to the "original" render-texture without
	 * breaking encapsulation.
	 */
	using OrigCb = std::function<void(tt::RenderTexture &)>;

	/**
	 * "Effects" texture callback type. Supplies const-references to `_orig_rt` and `_fx_rt`, and a reference
	 * to the `target` parameter of `full_lifecycle` to the callback. This is so the caller can customize how
	 * they draw to the final target using the "original" and "effects" render-textures.
	 */
	using FxCb = std::function<void(const tt::RenderTexture &, const tt::RenderTexture &, sf::RenderTarget &)>;

	std::string name;
	tt::RenderTexture _orig_rt, _fx_rt;
	bool auto_fx = true;
	OrigCb orig_cb;
	FxCb fx_cb;

public:
	static inline const FxCb DRAW_FX_RT = [](auto &, auto &fx_rt, auto &target)
	{
		target.draw(fx_rt.sprite);
	};

	/**
	 * The effects, in order, that will be applied to the "original"
	 * render-texture when `apply_fx()` is called.
	 */
	std::vector<std::unique_ptr<fx::Effect>> effects;

	Layer(const std::string &name, sf::Vector2u size, int antialiasing);

	/**
	 * @returns The name of this layer.
	 */
	const std::string &get_name() const { return name; }

	void orig_draw(const sf::Drawable &);
	void orig_display();

	/**
	 * Set the "original" callback. This is the callback that allows you to customize
	 * what is drawn to the "original" render-texture, A.K.A. before effects are applied.
	 */
	void set_orig_cb(const OrigCb &cb);

	/**
	 * Set whether effects are run when `full_lifecycle` is called.
	 */
	void set_auto_fx(const bool b);

	/**
	 * Set the "effects" callback. This is the callback that allows you to customize,
	 * using the "original" and "effects" render-textures, how to draw to the `target`
	 * passed to `full_lifecycle`.
	 */
	void set_fx_cb(const FxCb &cb);

	/**
	 * Runs the "full lifecycle" of the layer:
	 * - calls the "original" callback if given via `set_orig_cb`
	 * - if allowed (controlled by `set_auto_fx`), copies the "original" render-texture to the "effects" render-texture,
	 * then runs all effects on the "effects" render-texture
	 * - calls the "effects" callback if given via `set_fx_cb`
	 */
	void full_lifecycle(sf::RenderTarget &target);

	/**
	 * Copies the "original" render-texture to the "effects" render-texture and applies all `effects` on it.
	 */
	void apply_fx();

private:
};

} // namespace viz
