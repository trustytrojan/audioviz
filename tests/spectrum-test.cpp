#include <audioviz/Base.hpp>
#include <audioviz/StereoSpectrum.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

struct SpectrumTest : audioviz::Base
{
	const int fft_size = 3000;
	audioviz::ColorSettings color;
	audioviz::StereoSpectrum<audioviz::VerticalBar> ss;
	audioviz::fft::FrequencyAnalyzer fa;
	audioviz::fft::StereoAnalyzer sa;
	SpectrumTest(sf::Vector2u size, const std::string &media_url);
};

SpectrumTest::SpectrumTest(sf::Vector2u size, const std::string &media_url)
	: Base{size, new audioviz::FfmpegPopenMedia{media_url, size}},
	  ss{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size}
{
	ss.set_left_backwards(true);
	ss.set_bar_width(10);
	ss.set_bar_spacing(5);

	set_audio_frames_needed(fft_size);
	ss.configure_analyzer(sa); // only need to configure once since the spectrum isnt changing size

	set_audio_playback_enabled(true);
#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	auto &spectrum_layer = add_layer("spectrum");
	spectrum_layer.add_drawable(&ss);
	spectrum_layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			perform_fft(fa, sa);
			ss.update(sa);
		});

	start_in_window("spectrum-test");
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoi(argv[1]), std::stoi(argv[2])};
	SpectrumTest viz{size, argv[3]};
}
