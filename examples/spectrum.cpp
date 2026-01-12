#include <audioviz/Base.hpp>
#include <audioviz/StereoSpectrum.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

struct SpectrumTest : audioviz::Base
{
	const int fft_size = 3000;
	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable<audioviz::VerticalBar> spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{1};
	SpectrumTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

SpectrumTest::SpectrumTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size}
{
	spectrum.set_bar_width(10);
	spectrum.set_bar_spacing(5);

	set_audio_frames_needed(fft_size);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	// if we make this a layer, we can capture the full draw time
	add_final_drawable(spectrum);

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window(media, "spectrum-test");
}

void SpectrumTest::update(const std::span<const float> audio_buffer)
{
	capture_time("fft", aa.execute_fft(fa, audio_buffer, true));
	capture_time("ss_update", spectrum.update(fa, aa, 0));
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
