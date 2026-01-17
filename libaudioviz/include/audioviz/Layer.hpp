#pragma once

#include "audioviz/fx/TransformEffect.hpp"
#include <SFML/Graphics.hpp>
#include <audioviz/RenderTexture.hpp>
#include <audioviz/fx/PostProcessEffect.hpp>
#include <functional>
#include <vector>

/*
TODO: make a "Layer" interface, which the class below implements as a "PostProcessLayer".
this allows you to make an "OverlayLayer" which just draws its drawables to the target,
finally getting rid of the "add_final_drawable" crap in Base.

OR: don't make an interface, but have THIS class behave differently upon certain conditions.
    though this is less clear to the user at first glance.
*/

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
	 */
	using FxCb = std::function<void(const RenderTexture &, const RenderTexture &, sf::RenderTarget &)>;

	static inline const FxCb DRAW_FX_RT = [](auto &, auto &fx_rt, auto &target)
	{
		target.draw(fx_rt.sprite());
	};

	struct DrawCall
	{
		const sf::Drawable &drawable;
		const fx::TransformEffect *const transform_effect{};
		// const sf::RenderStates states; // this isn't needed anymore, but maybe it will be in the future
	};

private:
	std::string name;
	RenderTexture _orig_rt, _fx_rt;
	bool auto_fx{true};
	FxCb fx_cb{DRAW_FX_RT};

	/**
	 * The effects, in order, that will be applied to the "original"
	 * render-texture when `apply_fx()` is called.
	 */
	std::vector<const fx::PostProcessEffect *> effects;

	/**
	 * The draw calls to perform on this layer.
	 */
	std::vector<DrawCall> draws;

public:
	Layer(const std::string &name, sf::Vector2u size, unsigned antialiasing = 0);

	/**
	 * Add a draw call to perform on this layer. If more control over rendering is needed,
	 * use the `set_orig_cb`/`set_fx_cb` callback API instead.
	 */
	void add_draw(DrawCall);

	/**
	 * Add a post-processing effect to apply on the layer after all draw calls
	 * are executed.
	 */
	void add_effect(fx::PostProcessEffect *);

	/**
	 * @returns The name of this layer.
	 */
	inline const std::string &get_name() const { return name; }

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
};

} // namespace audioviz
