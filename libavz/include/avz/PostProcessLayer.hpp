#pragma once

#include "Layer.hpp"
#include "RenderTexture.hpp"
#include "fx/PostProcessEffect.hpp"
#include <functional>


namespace audioviz
{

class PostProcessLayer : public Layer
{
public:
	/**
	 * "Effects" texture callback type. Supplies references to `_orig_rt` and `_fx_rt`, and a reference
	 * to the `target` parameter of `full_lifecycle` to the callback. This is so the caller can customize how
	 * they draw to the final target using the "original" and "effects" render-textures.
	 */
	using FxCb = std::function<void(const RenderTexture &, const RenderTexture &, sf::RenderTarget &)>;

private:
	RenderTexture _orig_rt, _fx_rt;
	FxCb fx_cb;

	/**
	 * The effects, in order, that will be applied to the "original"
	 * render-texture when `apply_fx()` is called.
	 */
	std::vector<const fx::PostProcessEffect *> effects;

public:
	PostProcessLayer(const std::string &name, sf::Vector2u size, unsigned antialiasing = 0);

	/**
	 * Add a post-processing effect to apply on the layer after all draw calls
	 * are executed.
	 */
	void add_effect(fx::PostProcessEffect *);

	/**
	 * Set the "effects" callback. This is the callback that allows you to customize,
	 * using the "original" and "effects" render-textures, how to draw to the `target`
	 * passed to `full_lifecycle`.
	 *
	 * By default, the "effects" callback simply draws the "effects" render-texture to the
	 * `target` passed to `full_lifecycle`.
	 */
	inline void set_fx_cb(const FxCb &cb) { fx_cb = cb; }

	/**
	 * Runs the "full lifecycle" of the layer:
	 * - clears `_orig_rt`
	 * - calls the "original" callback if given via `set_orig_cb`
	 * - draws every `sf::Drawable` in `drawables`
	 * - if allowed (controlled by `set_auto_fx`), calls `apply_fx`
	 * - calls the "effects" callback if given via `set_fx_cb`
	 */
	virtual void render(sf::RenderTarget &target) override;
};

} // namespace audioviz
