#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

struct PolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();

	// 250ms audio window
	const int fft_size = sample_rate_hz / 4;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable<audioviz::VerticalBar> spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{1};

	PolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

PolarSpectrum::PolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url, 10}
{
	spectrum.set_bar_width(10);
	spectrum.set_bar_spacing(5);
	// fa.set_scale(audioviz::FrequencyAnalyzer::Scale::LINEAR);

	set_audio_frames_needed(fft_size);

	// Setup Polar Shader
	// We map the linear spectrum width (size.x) to a full circle
	audioviz::fx::Polar::setParameters(
		{size.x / 2.f, size.y / 2.f}, // Center
		(sf::Vector2f)size,			  // Dimensions of linear space
		size.y * 0.25f,				  // Base radius inner hole
		size.y * 0.5f				  // Max radius
	);

	add_final_drawable2(spectrum, &audioviz::fx::Polar::getShader());

	start_in_window(media, "polar-spectrum");
}

void PolarSpectrum::update(const std::span<const float> audio_buffer)
{
	aa.execute_fft(fa, audio_buffer, true);
	// spectrum.update_frequency_range(fa, aa, 0, 0, 200, sample_rate_hz, fft_size);
	// spectrum.update(fa, aa, 0);
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
