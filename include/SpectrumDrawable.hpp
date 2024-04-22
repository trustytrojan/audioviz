#pragma once

#include "FrequencySpectrum.hpp"
#include "VerticalPill.hpp"
#include "ColorUtils.hpp"

class SpectrumDrawable
{
public:
	enum class ColorMode
	{
		WHEEL,
		SOLID
	};

private:
	using FS = FrequencySpectrum;

	// spectrum parameters
	float multiplier = 4;

	// internal data
	FS fs;
	std::vector<float> spectrum;
	std::vector<VerticalPill> pills;

public:
	SpectrumDrawable(int fft_size);

	class
	{
		friend class SpectrumDrawable;
		int width = 10, spacing = 5;

	public:
		void set_width(int width) { this->width = width; }
		void set_spacing(int spacing) { this->spacing = spacing; }
		int get_width() const { return width; }
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
				return ColorUtils::hsvToRgb(index_ratio + h + wheel.time, s, v);
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

	void set_multiplier(float multiplier);
	const std::vector<float> &get_spectrum() const;

	// passthrough setters for FrequencySpectrum
	void set_fft_size(int fft_size);
	void set_interp_type(FS::InterpolationType interp_type);
	void set_scale(FS::Scale scale);
	void set_nth_root(int nth_root);
	void set_accum_method(FS::AccumulationMethod method);
	void set_window_func(FS::WindowFunction wf);

	void copy_channel_to_input(const float *const audio, int num_channels, int channel, bool interleaved);
	void copy_to_input(const float *const audio);
	void draw(sf::RenderTarget &target, sf::IntRect rect = {}, bool backwards = false);
};