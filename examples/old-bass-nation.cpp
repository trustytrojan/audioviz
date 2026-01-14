#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
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

constexpr float audio_duration_sec = 0.35;

struct StereoPolarSpectrum : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int fft_size = audio_duration_sec * sample_rate_hz;

	std::vector<float, aligned_allocator<float>> s, a;

	audioviz::ColorSettings cs;
	audioviz::SpectrumDrawable spectrum;
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa{sample_rate_hz, fft_size};
	audioviz::Interpolator ip;

	std::span<const float> audio_buffer;

	StereoPolarSpectrum(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

StereoPolarSpectrum::StereoPolarSpectrum(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  spectrum{{{}, (sf::Vector2i)size}, cs},
	  fa{fft_size},
	  media{media_url, 10}
{
#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("fft_size={} sample_rate_hz={}", fft_size, sample_rate_hz);

	spectrum.set_bar_width(1);
	spectrum.set_bar_spacing(0);
	spectrum.set_multiplier(6);
	cs.set_mode(audioviz::ColorSettings::Mode::SOLID);
	// fa.set_window_func({});

	set_audio_frames_needed(fft_size);

	sf::RenderStates polar_rs{&audioviz::fx::Polar::getShader()};

	auto &spectrum_layer = add_layer("spectrum");
	spectrum_layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			orig_rt.clear();

			auto do_work = [&](bool backwards, int channel, float angle, float duration_diff, sf::Color color)
			{
				const auto new_duration_sec = audio_duration_sec - duration_diff;
				const auto fft_size = new_duration_sec * sample_rate_hz;
				capture_time("fa.set_fft_size", fa.set_fft_size(fft_size));
				aa.set_fft_size(fft_size);
				const auto min_fft_index = audioviz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
				const auto max_fft_index = audioviz::util::bin_index_from_freq(125, sample_rate_hz, fft_size);

				spectrum.set_backwards(backwards);
				cs.set_solid_color(color);
				spectrum.update_bar_colors();

				a.resize(fft_size);
				capture_time(
					"strided_copy",
					audioviz::util::strided_copy(
						a, audio_buffer.first(fft_size * num_channels), num_channels, channel));
				capture_time("fft", aa.execute_fft(fa, a, true));
				s.assign(spectrum.get_bar_count(), 0);
				capture_time("compute_amps", aa.compute_amplitudes(fa));
				capture_time(
					"spread_out",
					audioviz::util::spread_out(
						s, {aa.compute_amplitudes(fa).data() + min_fft_index, max_fft_index - min_fft_index + 1}));
				capture_time("interpolate", ip.interpolate(s));
				capture_time("spectrum_update", spectrum.update(s));
				audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, angle, M_PI);
				orig_rt.draw(spectrum, polar_rs);
			};

			const std::array<sf::Color, 9> colors{
				sf::Color::Green,
				sf::Color::Cyan,
				sf::Color::Blue,
				sf::Color{146, 29, 255}, // purple
				sf::Color::Magenta,
				sf::Color::Red,
				sf::Color{255, 165, 0}, // orange
				sf::Color::Yellow,
				sf::Color::White};

			const auto delta_duration = 0.02f;
			const auto max_duration_diff = (colors.size() - 1) * delta_duration;

			// left channel
			for (int i = 0; i < colors.size(); ++i)
				capture_time("do_work", do_work(false, 0, M_PI / 2, max_duration_diff - i * delta_duration, colors[i]));

			// right channel
			for (int i = 0; i < colors.size(); ++i)
				capture_time("do_work", do_work(true, 1, -M_PI / 2, max_duration_diff - i * delta_duration, colors[i]));

			orig_rt.display();
		});

	start_in_window(media, "polar-spectrum");
}

void StereoPolarSpectrum::update(const std::span<const float> audio_buffer)
{
	this->audio_buffer = audio_buffer;
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	StereoPolarSpectrum viz{size, argv[3]};
}
