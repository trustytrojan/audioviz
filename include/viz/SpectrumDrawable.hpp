#pragma once

#include <SFML/Graphics.hpp>
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
public:
	enum class ColorMode
	{
		WHEEL,
		SOLID
	};

private:
	// spectrum parameters
	float multiplier = 4;

	// internal data
	std::vector<BarType> bars;
	sf::IntRect rect;
	bool backwards = false;

	struct
	{
		int width = 10, spacing = 5;
	} bar;

	// color stuff
	struct
	{
		ColorMode mode = ColorMode::WHEEL;
		sf::Color solid{255, 255, 255};

		/**
		 * @param index_ratio the ratio of your loop index (`i`) to the total number of bars to print (`bars.size()`)
		 */
		sf::Color get(const float index_ratio) const
		{
			switch (mode)
			{
			case ColorMode::WHEEL:
			{
				const auto [h, s, v] = wheel.hsv;
				return tt::hsv2rgb(index_ratio + h + wheel.time, s, v);
			}

			case ColorMode::SOLID:
				return solid;

			default:
				throw std::logic_error("SpectrumRenderer::color::get: default case hit");
			}
		}

		// wheel stuff
		struct
		{
			float time = 0, rate = 0;
			sf::Vector3f hsv{0.9, 0.7, 1};
			void increment_time() { time += rate; }
		} wheel;
	} color;

public:
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

	void set_color_mode(const ColorMode mode)
	{
		if (color.mode == mode)
			return;
		color.mode = mode;
		update_bar_colors();
	}

	void set_color_wheel_hsv(const sf::Vector3f hsv)
	{
		if (color.wheel.hsv == hsv)
			return;
		color.wheel.hsv = hsv;
		update_bar_colors();
	}

	void set_color_wheel_rate(const float rate)
	{
		if (color.wheel.rate == rate)
			return;
		color.wheel.rate = rate;
		// bar colors aren't updated here; `color_wheel_increment` must be called instead.
	}

	void color_wheel_increment()
	{
		if (color.wheel.rate == 0)
			return;
		color.wheel.increment_time();
		update_bar_colors();
	}

	void set_solid_color(const sf::Color rgb)
	{
		if (color.solid == rgb)
			return;
		color.solid = rgb;
		update_bar_colors();
	}

	void set_backwards(const bool b)
	{
		if (backwards == b)
			return;
		backwards = b;
		update_bars();
	}

	void update_bar_heights(const std::vector<float> &spectrum)
	{
		assert(spectrum.size() >= bars.size());
		for (int i = 0; i < (int)bars.size(); ++i)
			bars[i].setHeight(std::clamp(multiplier * rect.size.y * spectrum[i], 0.f, (float)rect.size.y));
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		for (const auto &bar : bars)
			target.draw(bar, states);
	}

	int bar_count() const { return bars.size(); }

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

	void update_bar_color(const int i) { bars[i].setFillColor(color.get((float)i / bars.size())); }
};

} // namespace viz
