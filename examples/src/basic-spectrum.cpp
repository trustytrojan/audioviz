#include "ExampleFramework.hpp"
#include <avz/gfx.hpp>
#include <avz/aligned_allocator.hpp>
#include <avz/analysis.hpp>
#include <avz/util.hpp>

using namespace avz::examples;

struct BasicSpectrum : ExampleBase
{
	int fft_size{};

	// buffer for extraction of first channel
	std::vector<float, aligned_allocator<float>> a;

	avz::ColorSettings color;
	avz::SpectrumDrawable spectrum;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	BasicSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  spectrum{{{}, (sf::Vector2i)size}, color},
		  fa{fft_size}
	{
		spectrum.set_bar_width(1);
		spectrum.set_bar_spacing(0);
		spectrum.set_multiplier(4);
		emplace_layer<avz::Layer>("spectrum").add_draw({spectrum});

		// remember, FFT takes N audio samples and returns N / 2 + 1 complex numbers
		fft_size = 2 * spectrum.get_bar_count();
		fa.set_fft_size(fft_size);
	}

	void update(std::span<const float> audio_buffer) override
	{
		// make sure we can fit one channel of audio
		a.resize(fft_size);

		// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
		capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));
		capture_time("amplitudes", aa.compute_amplitudes(fa));

		// finally, pass the data to SpectrumDrawable to draw to the screen!
		capture_time("spectrum_update", spectrum.update(aa.get_amplitudes()));
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(BasicSpectrum, "Spectrum visualization without preprocessing", 0.1f, viz.fft_size)
