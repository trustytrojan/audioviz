#include "ExampleFramework.hpp"
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <print>

using namespace audioviz::examples;

struct StereoPolarSpectrum : ExampleBase<StereoPolarSpectrum>
{
	const int fft_size;
	int min_fft_index, max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> s, a;

	audioviz::ColorSettings cs;
	audioviz::SpectrumDrawable spectrum_left, spectrum_right;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	audioviz::Interpolator ip;

	audioviz::fx::Polar polar_left, polar_right;

	StereoPolarSpectrum(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  spectrum_left{{{}, (sf::Vector2i)size}, cs},
		  spectrum_right{{{}, (sf::Vector2i)size}, cs},
		  fa{fft_size},
		  aa{sample_rate_hz, fft_size},
		  polar_left{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI},
		  polar_right{polar_left}
	{
		std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

		spectrum_left.set_bar_width(1);
		spectrum_right.set_bar_width(1);
		spectrum_left.set_bar_spacing(0);
		spectrum_right.set_bar_spacing(0);
		spectrum_left.set_multiplier(6);
		spectrum_right.set_multiplier(6);

		// Calculate frequency range (20-125 Hz)
		min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
		max_fft_index = audioviz::util::bin_index_from_freq(125, sample_rate_hz, fft_size);
		std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum_left.get_bar_count());

		polar_right.angle_start = -M_PI / 2;

		auto &spectrum_layer = emplace_layer<audioviz::Layer>("spectrum");
		spectrum_layer.add_draw({spectrum_left, &polar_left});
		spectrum_layer.add_draw({spectrum_right, &polar_right});
	}

	void update(std::span<const float> audio_buffer) override
	{
		auto process_channel = [&](bool backwards, int channel, audioviz::SpectrumDrawable &spectrum)
		{
			spectrum.set_backwards(backwards);
			a.resize(fft_size);
			capture_time("strided_copy", audioviz::util::extract_channel(a, audio_buffer, num_channels, channel));
			capture_time("fft", aa.execute_fft(fa, a));
			s.assign(spectrum.get_bar_count(), 0);
			capture_time(
				"spread_out",
				audioviz::util::spread_out(
					s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
			capture_time("interpolate", ip.interpolate(s));
			capture_time("spectrum_update", spectrum.update(s));
		};

		process_channel(false, 0, spectrum_left);
		process_channel(true, 1, spectrum_right);
	}
};

AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(
	StereoPolarSpectrum, "Stereo polar spectrum visualization with left and right channels", viz.fft_size)
