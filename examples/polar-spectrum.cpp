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

struct PolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int fft_size = audio_duration_sec * sample_rate_hz;
	int min_fft_index, max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> s, a;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{sample_rate_hz, fft_size};
	audioviz::Interpolator ip;

	PolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

PolarSpectrum::PolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url, 10}
{
#ifdef __linux__
	enable_profiler();
	set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrum.set_bar_width(2);
	spectrum.set_bar_spacing(0);
	spectrum.set_multiplier(6);

	set_audio_frames_needed(fft_size);

	// Calculate frequency range (0-250 Hz)
	min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
	max_fft_index = audioviz::util::bin_index_from_freq(250, sample_rate_hz, fft_size);
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());

	// Setup Polar Shader
	// We map the linear spectrum width (size.x) to a full circle
	audioviz::fx::Polar::setParameters(
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.y * 0.5f		// Max radius: 50% screen height
	);

	add_layer("spectrum").add_draw({spectrum, &audioviz::fx::Polar::getShader()});

	start_in_window(media, "polar-spectrum");
}

void PolarSpectrum::update(const std::span<const float> audio_buffer)
{
	a.resize(fft_size);
	capture_time("strided_copy", audioviz::util::extract_channel(a, audio_buffer, num_channels, 0));
	capture_time("fft", aa.execute_fft(fa, a));
	s.assign(spectrum.get_bar_count(), 0);
	capture_time(
		"spread_out",
		audioviz::util::spread_out(
			s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
	capture_time("interpolate", ip.interpolate(s));
	capture_time("spectrum_update", spectrum.update(s));
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	PolarSpectrum viz{size, argv[3]};
}
