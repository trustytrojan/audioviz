#pragma once

#include "tt/ColorUtils.hpp"

namespace viz
{

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

public:
	class
	{
		friend class SpectrumDrawable;
		int width = 10, spacing = 5;

	public:
		int get_spacing() const { return spacing; }
	} bar;

	// color stuff
	class
	{
		friend class SpectrumDrawable;
		ColorMode mode = ColorMode::WHEEL;
		sf::Color solid_rgb{255, 255, 255};

	public:
		void set_mode(const ColorMode mode) { this->mode = mode; }
		void set_solid_rgb(const sf::Color &rgb) { solid_rgb = rgb; }

		/**
		 * @param index_ratio the ratio of your loop index (aka `i`) to the total number of bars to print (aka `spectrum.size()`)
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
				return solid_rgb;

			default:
				throw std::logic_error("SpectrumRenderer::color::get: default case hit");
			}
		}

		// wheel stuff
		class
		{
			friend class SpectrumDrawable;
			float time = 0, rate = 0;
			// hue offset, saturation, value
			sf::Vector3f hsv{0.9, 0.7, 1};

		public:
			void set_rate(const float rate) { this->rate = rate; }
			void set_hsv(const sf::Vector3f &hsv) { this->hsv = hsv; }
			void increment() { time += rate; }
		} wheel;
	} color;

	// setters

	void set_multiplier(float multiplier)
	{
		this->multiplier = multiplier;
	}

	// set the area in which the spectrum will be drawn to
	void set_rect(const sf::IntRect &rect)
	{
		if (this->rect == rect)
			return;
		this->rect = rect;
		update_bars();
	}

	void set_bar_width(int width)
	{
		if (bar.width == width)
			return;
		bar.width = width;
		update_bars();
	}

	void set_bar_spacing(int spacing)
	{
		if (bar.spacing == spacing)
			return;
		bar.spacing = spacing;
		update_bars();
	}

	void set_backwards(bool b)
	{
		if (backwards == b)
			return;
		backwards = b;
		update_bars();
	}

	void update_bar_heights(const std::vector<float> &spectrum)
	{
		assert(spectrum.size() == bars.size());
		for (int i = 0; i < (int)spectrum.size(); ++i)
			bars[i].setHeight(std::clamp(multiplier * rect.height * spectrum[i], 0.f, (float)rect.height));
	}

	void draw(sf::RenderTarget &target, sf::RenderStates) const override
	{
		for (const auto &pill : bars)
			target.draw(pill);
	}

	int bar_count() const { return bars.size(); }

private:
	// call after changing any property of the spectrum/bars that will change their positions or colors
	void update_bars()
	{
		const int bar_count = rect.width / (bar.width + bar.spacing);
		bars.resize(bar_count);

		for (int i = 0; i < bar_count; ++i)
		{
			bars[i].setFillColor(color.get((float)i / bar_count));
			bars[i].setWidth(bar.width);

			// clang-format off
			const auto x = backwards
				? rect.left + rect.width - bar.width - i * (bar.width + bar.spacing)
				: rect.left + i * (bar.width + bar.spacing);
			// clang-format on

			bars[i].setPosition({x, rect.top + rect.height});
		}
	}
};

} // namespace viz
