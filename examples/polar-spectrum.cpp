#include "ExampleFramework.hpp"
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/util.hpp>

// #include <print>

using namespace audioviz::examples;

struct PolarSpectrum : ExampleBase<PolarSpectrum>
{
	const int fft_size;
	int min_fft_index, max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> s, a;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	audioviz::Interpolator ip;

	audioviz::fx::Polar polar;

	PolarSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size},
		  aa{sample_rate_hz, fft_size},
		  polar{
			  (sf::Vector2f)size, // Dimensions of linear space
			  size.y * 0.25f,	  // Base radius inner hole: 25% screen height
			  size.y * 0.5f,	  // Max radius: 50% screen height
			  0,				  // Angle start
			  2 * M_PI			  // Angle span
		  }
	{
		// std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

		spectrum.set_bar_width(2);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(6);

		// Calculate frequency range (20-250 Hz)
		min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
		max_fft_index = audioviz::util::bin_index_from_freq(250, sample_rate_hz, fft_size);
		// std::println(
		// 	"min_fft_index={} max_fft_index={} bar_count={}", min_fft_index, max_fft_index, spectrum.get_bar_count());

		emplace_layer<audioviz::Layer>("spectrum").add_draw({spectrum, &polar});
	}

	void update(std::span<const float> audio_buffer) override
	{
		a.resize(fft_size);
		capture_time("strided_copy", audioviz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		s.assign(spectrum.get_bar_count(), 0);
		// capture_time(
		// 	"spread_out",
		// 	audioviz::util::spread_out(
		// 		s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
		capture_time(
			"resample_spectrum",
			audioviz::util::resample_spectrum(s, aa.compute_amplitudes(fa), sample_rate_hz, fft_size, 20, 250, ip));
		capture_time("interpolate", ip.interpolate(s));
		capture_time("spectrum_update", spectrum.update(s));
	}
};

AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(PolarSpectrum, "Polar spectrum visualization with bass frequencies", viz.fft_size)
