#include "ExampleFramework.hpp"
#include <avz/SpectrumDrawable.hpp>
#include <avz/aligned_allocator.hpp>
#include <avz/fft/AudioAnalyzer.hpp>
#include <avz/fft/FrequencyAnalyzer.hpp>
#include <avz/fft/Interpolator.hpp>
#include <avz/fx/Polar.hpp>
#include <avz/util.hpp>

#include <SFML/Graphics.hpp>

using namespace avz::examples;

struct StereoPolarSpectrum : ExampleBase
{
	const int fft_size;

	std::vector<float, aligned_allocator<float, 32>> s, a;

	avz::ColorSettings cs;
	avz::SpectrumDrawable spectrum_left, spectrum_right;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;
	avz::Interpolator ip;

	avz::fx::Polar polar_left, polar_right;

	StereoPolarSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum_left{{{}, (sf::Vector2i)size}, cs},
		  spectrum_right{{{}, (sf::Vector2i)size}, cs},
		  fa{fft_size},
		  polar_left{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI},
		  polar_right{polar_left}
	{
		spectrum_left.set_bar_width(1);
		spectrum_right.set_bar_width(1);
		spectrum_left.set_bar_spacing(0);
		spectrum_right.set_bar_spacing(0);
		spectrum_left.set_multiplier(6);
		spectrum_right.set_multiplier(6);

		polar_right.angle_start = -M_PI / 2;

		auto &spectrum_layer = emplace_layer<avz::Layer>("spectrum");
		spectrum_layer.add_draw({spectrum_left, &polar_left});
		spectrum_layer.add_draw({spectrum_right, &polar_right});
	}

	void update(std::span<const float> audio_buffer) override
	{
		auto process_channel = [&](bool backwards, int channel, avz::SpectrumDrawable &spectrum)
		{
			spectrum.set_backwards(backwards);
			a.resize(fft_size);
			capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, channel));
			capture_time("fft", aa.execute_fft(fa, a));
			capture_time("amplitudes", aa.compute_amplitudes(fa));
			s.assign(spectrum.get_bar_count(), 0);
			capture_time(
				"resample_spectrum",
				avz::util::resample_spectrum(s, aa.get_amplitudes(), sample_rate_hz, fft_size, 20, 125, ip));
			capture_time("spectrum_update", spectrum.update(s));
		};

		process_channel(false, 0, spectrum_left);
		process_channel(true, 1, spectrum_right);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	StereoPolarSpectrum, "Stereo polar spectrum visualization with left and right channels", 0.25f, viz.fft_size)
