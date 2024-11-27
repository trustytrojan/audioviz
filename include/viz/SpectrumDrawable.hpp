#pragma once

#include "viz/ColorSettings.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <tt/ColorUtils.hpp>
#include <vector>

namespace viz
{

/**
 * A customizable frequency spectrum visualizer.
 * @tparam BarType A subclass of `sf::Shape` that implements `setWidth(float)` and `setHeight(float)`.
 */
template <typename BarType>
class SpectrumDrawable : public sf::Drawable
{
private:
	const ColorSettings &color;

	// spectrum parameters
	float multiplier{4};

	// internal data
	std::vector<BarType> bars;
	sf::IntRect rect;
	bool backwards = false;

	struct
	{
		int width{10}, spacing{5};
	} bar;

public:
	SpectrumDrawable(const ColorSettings &color, const bool backwards = false)
		: color{color},
		  backwards{backwards}
	{
	}

	SpectrumDrawable(const sf::IntRect &rect, const ColorSettings &color, const bool backwards = false)
		: rect{rect},
		  color{color},
		  backwards{backwards}
	{
		update_bars();
	}

	int get_bar_spacing() const { return bar.spacing; }

	void set_multiplier(const float multiplier) { this->multiplier = multiplier; }

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_bars();
	}

	void set_bar_width(const int width)
	{
		if (bar.width == width)
			return;
		bar.width = width;
		update_bars();
	}

	void set_bar_spacing(const int spacing)
	{
		if (bar.spacing == spacing)
			return;
		bar.spacing = spacing;
		update_bars();
	}

	void set_backwards(const bool b)
	{
		if (backwards == b)
			return;
		backwards = b;
		update_bars();
	}

	void update(const std::vector<float> &spectrum)
	{
		assert(spectrum.size() >= bars.size());
		for (int i = 0; i < (int)bars.size(); ++i)
		{
			bars[i].setFillColor(color.calculate_color((float)i / bars.size()));
			bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
		}
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &bar : bars)
			target.draw(bar, states);
	}

	inline int bar_count() const { return bars.size(); }

private:
	// call after changing any property of the spectrum/bars that will change their positions or colors
	void update_bars()
	{
		const int bar_count = rect.size.x / (bar.width + bar.spacing);
		bars.resize(bar_count);

		for (int i = 0; i < bar_count; ++i)
		{
			update_bar_color(i);
			bars[i].setWidth(bar.width);

			// clang-format off
			const auto x = backwards
				? rect.position.x + rect.size.x - bar.width - i * (bar.width + bar.spacing)
				: rect.position.x + i * (bar.width + bar.spacing);
			// clang-format on

			bars[i].setPosition({x, rect.position.y + rect.size.y});
		}
	}

	// only update bar colors
	void update_bar_colors()
	{
		for (int i = 0; i < (int)bars.size(); ++i)
			update_bar_color(i);
	}

	inline void update_bar_color(const int i) { bars[i].setFillColor(color.calculate_color((float)i / bars.size())); }
};

} // namespace viz
