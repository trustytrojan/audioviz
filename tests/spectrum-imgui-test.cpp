#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <iostream>

struct SpectrumImGuiTest : audioviz::Base
{
	const int fft_size = 3000;
	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable<audioviz::VerticalBar> spectrum;
	audioviz::fft::FrequencyAnalyzer fa;
	audioviz::fft::AudioAnalyzer aa;

	SpectrumImGuiTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

SpectrumImGuiTest::SpectrumImGuiTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  aa{2}
{
	spectrum.set_bar_width(10);
	spectrum.set_bar_spacing(5);

	set_audio_frames_needed(fft_size);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	add_final_drawable(spectrum);

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window_with_imgui(media, "spectrum-imgui-test");
}

void SpectrumImGuiTest::update(const std::span<const float> audio_buffer)
{
	// Draw ImGui controls right here in update
	ImGui::Begin("Spectrum Controls");
	spectrum.draw_imgui();
	ImGui::End();

	// now that spectrum values CAN CHANGE VIA IMGUI, we NEED
	// to ALWAYS configure the analyzer!
	spectrum.configure_analyzer(aa);
	aa.analyze(fa, audio_buffer.data(), true);
	spectrum.update(aa.get_spectrum_data(0));
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	SpectrumImGuiTest viz{size, argv[3]};
}