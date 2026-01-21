#include "ExampleFramework.hpp"
#include <avz/SpectrumDrawable.hpp>
#include <avz/aligned_allocator.hpp>
#include <avz/analysis.hpp>
#include <avz/util.hpp>

using namespace avz::examples;

struct RangedSpectrum : ExampleBase
{
	const int fft_size;

	std::vector<float, aligned_allocator<float, 32>> a, s;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	avz::Interpolator ip;

	RangedSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size}
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);

		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum});
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

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	RangedSpectrum, "Spectrum visualization with frequency range filtering (20-250 Hz)", 0.25f, viz.fft_size)
