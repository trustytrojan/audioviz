#pragma once

#include "audioviz/fx/TransformEffect.hpp"
#include <SFML/Graphics.hpp>
#include <audioviz/RenderTexture.hpp>
#include <audioviz/fx/PostProcessEffect.hpp>
#include <functional>
#include <vector>

namespace audioviz
{

struct DrawCall
{
	const sf::Drawable &drawable;
	const fx::TransformEffect *const transform_effect{};

	// this isn't needed anymore, but maybe it will be in the future
	// const sf::RenderStates states;
};

class Layer
{
	std::string name;
	std::vector<DrawCall> draws;

public:
	inline Layer(const std::string &name)
		: name{name}
	{
	}

	inline const std::string &get_name() const { return name; }
	inline void add_draw(DrawCall dc) { draws.emplace_back(dc); }

	virtual void render(sf::RenderTarget &target)
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
};

class PostProcessLayer : public Layer
{
public:
	/**
	 * "Effects" texture callback type. Supplies references to `_orig_rt` and `_fx_rt`, and a reference
	 * to the `target` parameter of `full_lifecycle` to the callback. This is so the caller can customize how
	 * they draw to the final target using the "original" and "effects" render-textures.
	 */
	using FxCb = std::function<void(const RenderTexture &, const RenderTexture &, sf::RenderTarget &)>;

	static inline const FxCb DRAW_FX_RT = [](auto &, auto &fx_rt, auto &target)
	{
		target.draw(fx_rt.sprite());
	};

private:
	RenderTexture _orig_rt, _fx_rt;
	FxCb fx_cb{DRAW_FX_RT};

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
