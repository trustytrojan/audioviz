#include "audioviz/Player.hpp"
#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <print>

constexpr float audio_duration_sec = 0.15;

struct StereoPolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int fft_size = audio_duration_sec * sample_rate_hz;
	int min_fft_index, max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> s, a;

	audioviz::ColorSettings cs;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{sample_rate_hz, fft_size};
	audioviz::Interpolator ip;

	std::span<const float> audio_buffer;

	StereoPolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

StereoPolarSpectrum::StereoPolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, cs},
	  fa{fft_size},
	  media{media_url}
{
#ifdef __linux__
	enable_profiler();
	set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);
	spectrum.set_multiplier(6);

	// Calculate frequency range (0-250 Hz)
	min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
	max_fft_index = audioviz::util::bin_index_from_freq(125, sample_rate_hz, fft_size);
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());

	sf::RenderStates polar_rs{&audioviz::fx::Polar::getShader()};

	auto &spectrum_layer = add_layer("spectrum");
	spectrum_layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			orig_rt.clear();

			auto process_channel = [&](bool backwards, int channel, float angle)
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
				audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, angle, M_PI);
				orig_rt.draw(spectrum, polar_rs);
			};

			process_channel(false, 0, M_PI / 2);
			process_channel(true, 1, -M_PI / 2);

			orig_rt.display();
		});
}

void StereoPolarSpectrum::update(const std::span<const float> audio_buffer)
{
	this->audio_buffer = audio_buffer;
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	StereoPolarSpectrum viz{size, argv[3]};
	audioviz::Player{viz, viz.media, 60, viz.fft_size}.start_in_window(argv[0]);
}
