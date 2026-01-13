#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/VerticalBar.hpp>
#include <audioviz/fft/AudioAnalyzer_new.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <print>

struct PolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};

	// 250ms audio window
	const int fft_size = sample_rate_hz / 4;
	int max_fft_index;

	std::vector<float> s, a;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable<audioviz::VerticalBar> spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer_new aa{sample_rate_hz, fft_size};
	audioviz::Interpolator ip;

	PolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

PolarSpectrum::PolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url}
{
	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);

	set_audio_frames_needed(fft_size);

	// Calculate frequency range (0-250 Hz)
	max_fft_index = audioviz::util::bin_index_from_freq(250, sample_rate_hz, fa.get_fft_size());
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrum.get_bar_count());

	// Setup Polar Shader
	// We map the linear spectrum width (size.x) to a full circle
	audioviz::fx::Polar::setParameters(
		(sf::Vector2f)size,			  // Dimensions of linear space
		size.y * 0.25f,				  // Base radius inner hole
		size.y * 0.5f				  // Max radius
	);

	add_final_drawable2(spectrum, &audioviz::fx::Polar::getShader());

	start_in_window(media, "polar-spectrum");
}

void PolarSpectrum::update(const std::span<const float> audio_buffer)
{
	a.resize(fft_size);
	audioviz::util::strided_copy(a, audio_buffer, num_channels, 0);
	aa.execute_fft(fa, a, false);
	s.assign(spectrum.get_bar_count(), 0);
	// Spread out the frequency range (0-250 Hz) into spectrum bars
	audioviz::util::spread_out(s, {aa.compute_amplitudes(fa).data(), max_fft_index});
	ip.interpolate(s);
	spectrum.update(s);
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
