#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

using namespace avz::examples;

struct PolarSpectrum : ExampleBase
{
	const int fft_size;

	std::vector<float> s, a;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;
	avz::Interpolator ip;

	avz::fx::Polar polar;

	PolarSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size},
		  polar{
			  (sf::Vector2f)size, // Dimensions of linear space
			  size.y * 0.25f,	  // Base radius inner hole: 25% screen height
			  size.y * 0.5f,	  // Max radius: 50% screen height
			  M_PI / 2,		  // Angle start
			  2 * M_PI			  // Angle span
		  }
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(6);
		spectrum.set_use_gs(true);

		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum, &polar});
	}

	void update(std::span<const float> audio_buffer) override
	{
		a.resize(fft_size);
		capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		capture_time("amplitudes", aa.compute_amplitudes(fa));
		s.assign(spectrum.get_bar_count(), 0);
		capture_time(
			"resample_spectrum",
			avz::util::resample_spectrum(s, aa.get_amplitudes(), sample_rate_hz, fft_size, 20, 250, ip));
		capture_time("spectrum_update", spectrum.update(s));
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(PolarSpectrum, "Polar spectrum visualization with bass frequencies", 0.15f, viz.fft_size)
