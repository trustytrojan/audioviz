#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/aligned_allocator.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <print>

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

constexpr float audio_duration_sec = 0.1;

struct RangedSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz{media.audio_sample_rate()};
	int num_channels{media.audio_channels()};
	const int fft_size = audio_duration_sec * sample_rate_hz;
	int max_fft_index;

	std::vector<float, aligned_allocator<float, 32>> a, s;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{sample_rate_hz, fft_size};

	audioviz::Interpolator ip;

	RangedSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

RangedSpectrum::RangedSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url}
{
	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);

	set_audio_frames_needed(fft_size);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	add_layer("spectrum").add_draw({spectrum});

	max_fft_index = audioviz::util::bin_index_from_freq(250, sample_rate_hz, fa.get_fft_size());
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());

	start_in_window(media, "spectrum-test");
}

void RangedSpectrum::update(const std::span<const float> audio_buffer)
{
	a.resize(fft_size);
	capture_time("strided_copy", audioviz::util::strided_copy(a, audio_buffer, num_channels, 0));
	capture_time("fft", aa.execute_fft(fa, a));
	s.assign(spectrum.get_bar_count(), 0);
	// spread out into s only our desired frequency range
	capture_time("spread_out", audioviz::util::spread_out(s, {aa.compute_amplitudes(fa).data(), max_fft_index}));
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
	RangedSpectrum viz{size, argv[3]};
}
