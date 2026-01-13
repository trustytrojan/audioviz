#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/SpectrumDrawable_new.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
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

constexpr float audio_duration_sec = 0.25;

struct SpectrumTest : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz{media.audio_sample_rate()};
	const int fft_size = audio_duration_sec * sample_rate_hz;
	int max_fft_index;

	std::vector<float> s;

	audioviz::ColorSettings color;
	// audioviz::SpectrumDrawable<audioviz::VerticalBar> spectrum;
	audioviz::SpectrumDrawable_new spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{1};

	SpectrumTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

SpectrumTest::SpectrumTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url, 10}
{
	std::println("fft_size: {}", fft_size);

	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);
	// spectrum.set_bar_count(100);
	// fa.set_scale(audioviz::FrequencyAnalyzer::Scale::LINEAR);

	set_audio_frames_needed(fft_size);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	// if we make this a layer, we can capture the full draw time
	// add_final_drawable(spectrum);
	add_layer("spectrum", 4).add_draw({spectrum});

	max_fft_index = audioviz::util::bin_index_from_freq(350, sample_rate_hz, fa.get_fft_output_size());
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());

	start_in_window(media, "spectrum-test");
}

void SpectrumTest::update(const std::span<const float> audio_buffer)
{
	capture_time("fft", aa.execute_fft(fa, audio_buffer, true));
	// capture_time("spectrum_update", spectrum.update(fa, aa, 0));

	const auto &fft_amplitudes = aa.get_channel_data(0).fft_amplitudes;
	const auto bar_count = spectrum.get_bar_count();
	s.assign(bar_count, 0);

	const auto increment = (float)bar_count / (float)max_fft_index;

	for (int i = 0; i < max_fft_index; ++i)
		s[i * increment] = fft_amplitudes[i];

	capture_time("interpolate", fa.interpolate(s));

	capture_time("spectrum_update", spectrum.update(s));

	// fa.bin_pack(s, aa.get_channel_data(0).fft_amplitudes);
	// spectrum.update(aa.get_channel_data(0).fft_amplitudes);
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	SpectrumTest viz{size, argv[3]};
}
