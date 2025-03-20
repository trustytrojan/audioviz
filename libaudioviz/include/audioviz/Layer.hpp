#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/RenderTexture.hpp>
#include <audioviz/fx/Effect.hpp>
#include <functional>
#include <vector>

namespace audioviz
{

class Layer
{
public:
	/**
	 * "Original" texture callback type. Supplies a reference to `_orig_rt` to the callback.
	 * This is so the caller can customize what is drawn to the "original" render-texture without
	 * breaking encapsulation.
	 */
	using OrigCb = std::function<void(RenderTexture &)>;

	/**
	 * "Effects" texture callback type. Supplies references to `_orig_rt` and `_fx_rt`, and a reference
	 * to the `target` parameter of `full_lifecycle` to the callback. This is so the caller can customize how
	 * they draw to the final target using the "original" and "effects" render-textures.
	 *
	 * FYI if `auto_fx` is `false` then the contents of `fx_rt` passed to this callback are undefined.
	 * Use `orig_rt` in this case since you did not want to apply effects on this layer.
	 *
	 * EDIT: had to make the first two params non-const refs due to sol2 copying const-ref objects by default
	 */
	using FxCb = std::function<void(RenderTexture &, RenderTexture &, sf::RenderTarget &)>;

	static inline const FxCb DRAW_FX_RT = [](auto &, auto &fx_rt, auto &target)
	{
		target.draw(fx_rt.sprite());
	};

private:
	std::string name;
	RenderTexture _orig_rt, _fx_rt;
	bool auto_fx = true;
	OrigCb orig_cb;
	FxCb fx_cb = DRAW_FX_RT;

public:
	/**
	 * The effects, in order, that will be applied to the "original"
	 * render-texture when `apply_fx()` is called.
	 */
	std::vector<const fx::Effect *> effects;

	/**
	 * The drawables to draw on this layer. If they have state that needs to be updated
	 * over time, do so in a lambda function passed to `set_orig_cb`.
	 */
	std::vector<const sf::Drawable *> drawables;

	Layer(const std::string &name, sf::Vector2u size, int antialiasing);

	/**
	 * @returns The name of this layer.
	 */
	inline const std::string &get_name() const { return name; }

	inline const RenderTexture &fx_rt() const { return _fx_rt; }

	/**
	 * Calls `draw(...)` on the "original" render-texture. Useful if you want to prepopulate
	 * a layer with a static image without re-rendering it every frame.
	 */
	void orig_draw(const sf::Drawable &);

	/**
	 * Calls `display()` on the "original" render-texture. Useful if you want to prepopulate
	 * a layer with a static image without re-rendering it every frame.
	 */
	void orig_display();

	/**
	 * Set the "original" callback. This is the callback that allows you to customize
	 * what is drawn to the "original" render-texture, aka before effects are applied.
	 */
	void set_orig_cb(const OrigCb &);

	/**
	 * Set whether effects are run when `full_lifecycle` is called.
	 */
	void set_auto_fx(bool);

	/**
	 * Set the "effects" callback. This is the callback that allows you to customize,
	 * using the "original" and "effects" render-textures, how to draw to the `target`
	 * passed to `full_lifecycle`.
	 *
	 * By default, the "effects" callback simply draws the "effects" render-texture to the
	 * `target` passed to `full_lifecycle`.
	 *
	 * FYI if `auto_fx` is `false` then the contents of `fx_rt` passed to this callback are undefined.
	 * Use `orig_rt` in this case since you did not want to apply effects on this layer.
	 */
	void set_fx_cb(const FxCb &);

	/**
	 * Runs the "full lifecycle" of the layer:
	 * - clears `_orig_rt`
	 * - calls the "original" callback if given via `set_orig_cb`
	 * - draws every `sf::Drawable` in `drawables`
	 * - if allowed (controlled by `set_auto_fx`), calls `apply_fx`
	 * - calls the "effects" callback if given via `set_fx_cb`
	 */
	void full_lifecycle(sf::RenderTarget &target);

	/**
	 * Copies the "original" render-texture to the "effects" render-texture and applies all `effects` on it.
	 *
	 * This is called by `full_lifecycle` unless `set_auto_fx(false)` is called.
	 *
	 * Calling this method manually can be useful if you want to prepopulate a layer with a static image
	 * without re-rendering it every frame.
	 */
	void apply_fx();
};

} // namespace audioviz
