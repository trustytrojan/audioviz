#pragma once

#include "tt/ColorUtils.hpp"
#include "tt/FrequencySpectrum.hpp"

#include "VerticalPill.hpp"

namespace viz
{

template <typename BarType>
class SpectrumDrawable : public sf::Drawable
{
	// static_assert(std::is_base_of_v<SpectrumBar, BarType>, "BarType must be a subtype of SpectrumBar");

public:
	enum class ColorMode
	{
		WHEEL,
		SOLID
	};

private:
	using FS = tt::FrequencySpectrum;

	// spectrum parameters
	float multiplier = 4;

	// internal data
	FS fs;
	std::vector<float> spectrum;
	std::vector<BarType> bars;
	sf::IntRect rect;
	bool backwards = false;

public:
	SpectrumDrawable(const int fft_size)
		: fs(fft_size) {}

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
		// sanity check
		assert(spectrum.size() == bars.size());
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

	// call after changing any property of the spectrum/bars that will change their positions or colors
	void update_bars()
	{
		// sanity check
		assert(spectrum.size() == bars.size());

		const int bar_count = rect.width / (bar.width + bar.spacing);
		spectrum.resize(bar_count);
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

	void do_fft(const float *const audio, int num_channels, int channel, bool interleaved)
	{
		fs.copy_channel_to_input(audio, num_channels, channel, interleaved);
		do_fft();
	}

	void do_fft(const float *const audio)
	{
		fs.copy_to_input(audio);
		do_fft();
	}

	const std::vector<float> &data() const { return spectrum; }

	void draw(sf::RenderTarget &target, sf::RenderStates) const override
	{
		// sanity check
		assert(spectrum.size() == bars.size());

		for (const auto &pill : bars)
			target.draw(pill);
	}

	// passthrough setters for FrequencySpectrum
	void set_fft_size(int fft_size) { fs.set_fft_size(fft_size); }
	void set_interp_type(FS::InterpolationType interp_type) { fs.set_interp_type(interp_type); }
	void set_scale(FS::Scale scale) { fs.set_scale(scale); }
	void set_nth_root(int nth_root) { fs.set_nth_root(nth_root); }
	void set_accum_method(FS::AccumulationMethod method) { fs.set_accum_method(method); }
	void set_window_func(FS::WindowFunction wf) { fs.set_window_func(wf); }

private:
	void do_fft()
	{
		// sanity check
		assert(spectrum.size() == bars.size());

		const int bar_count = spectrum.size();

		fs.render(spectrum);

		for (int i = 0; i < bar_count; ++i)
		{
			// calculate new pill height (sometimes spectrum output is negative)
			auto height = multiplier * rect.height * std::max(0.f, spectrum[i]);

			// don't go over the target's height
			height = std::min((float)rect.height, height);

			bars[i].setHeight(height);
		}
	}
};

} // namespace viz
