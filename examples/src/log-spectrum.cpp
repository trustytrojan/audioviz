#include "ExampleFramework.hpp"
#include <avz/gfx.hpp>
#include <avz/analysis.hpp>

using namespace avz::examples;

struct LogSpectrum : ExampleBase
{
	int fft_size{};

	// audio, spectrum
	std::vector<float> a, s;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	// needed to logarithmically pack fft_size spectral samples into the spectrum's bars
	avz::BinPacker bp;

	// needed to interpolate the gaps left by BinPacker's spreading out of bins
	avz::Interpolator ip;

	LogSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size}
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);
		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum});

		fft_size = 2 * spectrum.get_bar_count();
		fa.set_fft_size(fft_size);

		// logarithmically scale bin indices (frequencies)
		bp.set_scale(avz::BinPacker::Scale::LOG);

		// when multiple values go to a bin, accumulate them using std::max()
		// for fun, change this to SUM and see what happens
		bp.set_accum_method(avz::BinPacker::AccumulationMethod::MAX);
	}

	void update(std::span<const float> audio_buffer) override
	{
		// make sure we can fit one channel of audio
		a.resize(fft_size);

		// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
		capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		capture_time("amplitudes", aa.compute_amplitudes(fa));

		// make sure we can fit all the spectrum bars
		// it's not necessary here, but assign all zero in case the spectrum bar count changes dynamically
		s.assign(spectrum.get_bar_count(), 0);

		// pack FFT amplitudes into a smaller set of "bins" (our spectrum bars!)
		capture_time("bin_pack", bp.bin_pack(s, aa.get_amplitudes()));

		// there will be gaps caused by BinPacker because we logarithmically spread the output bins
		// interpolate between them to make a nice looking curve
		capture_time("interpolate", ip.interpolate(s));

		// finally, pass the data to SpectrumDrawable to draw to the screen!
		capture_time("spectrum_update", spectrum.update(s));
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(LogSpectrum, "Spectrum visualization with logarithmic frequency scaling", 0.1f, viz.fft_size)
