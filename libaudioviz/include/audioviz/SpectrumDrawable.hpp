#pragma once

#include <SFML/Graphics.hpp>
#include <audioviz/ColorSettings.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <vector>

namespace audioviz
{

/**
 * A customizable frequency spectrum visualizer.
 * @tparam BarType A subclass of `sf::Shape` that implements `setWidth(float)` and `setHeight(float)`.
 */
template <typename BarType>
class SpectrumDrawable : public sf::Drawable
{
	const ColorSettings &color;

	// spectrum parameters
	float multiplier{4};

	// internal data
	std::vector<BarType> bars;
	sf::IntRect rect;
	bool backwards{};
	bool debug_rect{};

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

	inline void set_debug_rect(bool b) { debug_rect = b; }
	inline int get_bar_spacing() const { return bar.spacing; }
	inline void set_multiplier(const float multiplier) { this->multiplier = multiplier; }

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

	/**
	 * Use this to resize the analyzer's spectrum vector to be the same length
	 * as the number of bars we are rendering. This is REQUIRED before calling `update`.
	 */
	void configure_analyzer(fft::AudioAnalyzer &aa) { aa.resize(bars.size()); }

	/**
	 * Update the heights of the bars using the provided `spectrum` data.
	 * Requires that `spectrum.size() >= bars.size()`. Call `configure_analyzer`
	 * before performing FFT to satisfy that requirement.
	 */
	void update(const std::vector<float> &spectrum)
	{
		assert(spectrum.size() >= bars.size());
		if (color.wheel.rate != 0)
			for (int i = 0; i < (int)bars.size(); ++i)
			{
				// setFillColor is expensive, only call it if we need to!
				update_bar_color(i);
				bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
			}
		else
			for (int i = 0; i < (int)bars.size(); ++i)
				bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &bar : bars)
			target.draw(bar, states);
		if (debug_rect)
		{
			// shows the rect of the object
			sf::RectangleShape r{sf::Vector2f{rect.size}};
			r.setFillColor(sf::Color::Transparent);
			r.setOutlineThickness(1);
			r.setPosition(sf::Vector2f{rect.position});

			// shows the origin of the object
			sf::CircleShape c{5};
			c.setPosition(sf::Vector2f{rect.position});

			// draw with states,
			target.draw(r, states);
			target.draw(c, states);

			// then draw without any states in red, to show how any render states may have transformed the object
			r.setOutlineColor(sf::Color::Red);
			c.setFillColor(sf::Color::Red);
			target.draw(r);
			target.draw(c);
		}
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

	void update_bar_color(const int i) { bars[i].setFillColor(color.calculate_color((float)i / bars.size())); }
};

} // namespace audioviz
