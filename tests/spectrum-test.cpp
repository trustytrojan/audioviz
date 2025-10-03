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
	audioviz::StereoSpectrum<audioviz::VerticalBar> ss;
	audioviz::fft::FrequencyAnalyzer fa;
	audioviz::fft::StereoAnalyzer sa;
	SpectrumTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

SpectrumTest::SpectrumTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  ss{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size}
{
	ss.set_left_backwards(true);
	ss.set_bar_width(10);
	ss.set_bar_spacing(5);
	ss.configure_analyzer(sa); // only need to configure once since the spectrum isnt changing bar count

	set_audio_frames_needed(fft_size);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	// if we make this a layer, we can capture the full draw time
	add_final_drawable(ss);

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window(media, "spectrum-test");
}

void SpectrumTest::update(const std::span<const float> audio_buffer)
{
	capture_time("fft", sa.analyze(fa, audio_buffer.data(), true));
	capture_time("ss_update", ss.update(sa));
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
