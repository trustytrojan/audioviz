#include "ExampleFramework.hpp"
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/util.hpp>

#include <print>

using namespace audioviz::examples;

struct RangedSpectrum : ExampleBase<RangedSpectrum>
{
	const int fft_size;
	int max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> a, s;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;

	audioviz::Interpolator ip;

	RangedSpectrum(const ExampleConfig& config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size},
		  aa{sample_rate_hz, fft_size}
	{
		std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);

		emplace_layer<audioviz::Layer>("spectrum").add_draw({spectrum});

		max_fft_index = audioviz::util::bin_index_from_freq(250, sample_rate_hz, fa.get_fft_size());
		std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());
	}

	void update(std::span<const float> audio_buffer) override
	{
		a.resize(fft_size);
		capture_time("strided_copy", audioviz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		s.assign(spectrum.get_bar_count(), 0);
		// spread out into s only our desired frequency range
		capture_time("spread_out", audioviz::util::spread_out(s, {aa.compute_amplitudes(fa).data(), max_fft_index}));
		capture_time("interpolate", ip.interpolate(s));
		capture_time("spectrum_update", spectrum.update(s));
	}
};

AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(
	RangedSpectrum,
	"Spectrum visualization with frequency range filtering (20-250 Hz)",
	viz.fft_size
)
