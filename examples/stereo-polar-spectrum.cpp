#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable_new.hpp>
#include <audioviz/fft/AudioAnalyzer_new.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
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

constexpr float audio_duration_sec = 0.15;

struct PolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int fft_size = audio_duration_sec * sample_rate_hz;
	int min_fft_index, max_fft_index;

	std::vector<float> s, a;

	audioviz::ColorSettings color;
	audioviz::SpectrumDrawable_new spectrumL, spectrumR;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer_new aa{sample_rate_hz, fft_size};
	audioviz::Interpolator ip;

	PolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

PolarSpectrum::PolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrumL{{{}, (sf::Vector2i)size}, color},
	  spectrumR{{{}, (sf::Vector2i)size}, color},
	  fa{fft_size},
	  media{media_url}
{
#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrumL.set_bar_width(1);
	spectrumL.set_bar_spacing(0);
	spectrumR.set_bar_width(1);
	spectrumR.set_bar_spacing(0);
	spectrumR.set_backwards(true);

	spectrumL.set_multiplier(6);
	spectrumR.set_multiplier(6);

	set_audio_frames_needed(fft_size);

	// Calculate frequency range (0-250 Hz)
	min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
	max_fft_index = audioviz::util::bin_index_from_freq(125, sample_rate_hz, fft_size);
	std::println("max_fft_index={} bar_count={}", max_fft_index, spectrumL.get_bar_count());

	// Setup Polar Shader
	// We map the linear spectrum width (size.x) to a full circle
	audioviz::fx::Polar::setParameters(
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.y * 0.5f,		// Max radius: 50% screen height
		M_PI / 2,			// Start angle: shift a quarter circle
		M_PI				// Angle span: half a cicle
	);

	// add_layer("spectrum").add_draw({spectrumL, &audioviz::fx::Polar::getShader()});
	sf::RenderStates polar_rs{&audioviz::fx::Polar::getShader()};

	auto &spectrum_layer = add_layer("spectrum");
	spectrum_layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			orig_rt.clear();
			audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI);
			orig_rt.draw(spectrumL, polar_rs);
			audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, -M_PI / 2, M_PI);
			orig_rt.draw(spectrumR, polar_rs);
			orig_rt.display();
		});

	start_in_window(media, "polar-spectrum");
}

void PolarSpectrum::update(const std::span<const float> audio_buffer)
{
	a.resize(fft_size);
	capture_time("strided_copy", audioviz::util::strided_copy(a, audio_buffer, num_channels, 0));
	capture_time("fft", aa.execute_fft(fa, a, true));
	s.assign(spectrumL.get_bar_count(), 0);
	capture_time(
		"spread_out",
		audioviz::util::spread_out(
			s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
	capture_time("interpolate", ip.interpolate(s));
	capture_time("spectrum_update", spectrumL.update(s));

	a.resize(fft_size);
	capture_time("strided_copy", audioviz::util::strided_copy(a, audio_buffer, num_channels, 1));
	capture_time("fft", aa.execute_fft(fa, a, true));
	s.assign(spectrumR.get_bar_count(), 0);
	capture_time(
		"spread_out",
		audioviz::util::spread_out(
			s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
	capture_time("interpolate", ip.interpolate(s));
	capture_time("spectrum_update", spectrumR.update(s));
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
