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

constexpr float audio_duration_sec = 0.25;

struct LayerData
{
	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	std::vector<float, aligned_allocator<float>> s, a;
	audioviz::SpectrumDrawable spectrum_left;
	audioviz::SpectrumDrawable spectrum_right;

	LayerData(int fft_size, int sample_rate, sf::Vector2u size, const audioviz::ColorSettings &cs)
		: fa{fft_size},
		  aa{sample_rate, fft_size},
		  spectrum_left{{{}, (sf::Vector2i)size}, cs},
		  spectrum_right{{{}, (sf::Vector2i)size}, cs}
	{
		a.resize(fft_size);
	}
};

struct OldBassNation : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels{media.audio_channels()};
	const int max_fft_size = audio_duration_sec * sample_rate_hz;

	std::vector<LayerData> layers;

	audioviz::ColorSettings cs;
	audioviz::Interpolator ip;

	std::span<const float> audio_buffer;

	OldBassNation(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

OldBassNation::OldBassNation(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  media{media_url, 15}
{
#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	std::println("max_fft_size={} sample_rate_hz={}", max_fft_size, sample_rate_hz);

	cs.set_mode(audioviz::ColorSettings::Mode::SOLID);

	set_audio_frames_needed(max_fft_size);

	sf::RenderStates polar_rs{&audioviz::fx::Polar::getShader()};

	static const std::array<sf::Color, 9> colors{
		sf::Color::Green,
		sf::Color::Cyan,
		sf::Color::Blue,
		sf::Color{146, 29, 255}, // purple
		sf::Color::Magenta,
		sf::Color::Red,
		sf::Color{255, 165, 0}, // orange
		sf::Color::Yellow,
		sf::Color::White //
	};

	const auto delta_duration = 0.015f;
	const auto max_duration_diff = (colors.size() - 1) * delta_duration;

	layers.reserve(colors.size());
	for (int i = 0; i < colors.size(); ++i)
	{
		float duration_diff = max_duration_diff - i * delta_duration;
		const auto new_duration_sec = audio_duration_sec - duration_diff;
		const int new_fft_size = new_duration_sec * sample_rate_hz;

		layers.emplace_back(new_fft_size, sample_rate_hz, size, cs);
		auto &l = layers.back();

		l.fa.set_window_func(audioviz::FrequencyAnalyzer::WindowFunction::Blackman);

		auto setup = [&](audioviz::SpectrumDrawable &s, bool prev)
		{
			s.set_bar_width(1);
			s.set_bar_spacing(0);
			s.set_multiplier(6);
			s.set_backwards(prev);
			s.update_bar_colors();
		};

		cs.set_solid_color(colors[i]);
		setup(l.spectrum_left, false);
		setup(l.spectrum_right, true);

		l.s.resize(l.spectrum_left.get_bar_count());
	}

	auto &spectrum_layer = add_layer("spectrum");
	spectrum_layer.set_orig_cb(
		[&, polar_rs](auto &orig_rt) mutable
		{
			orig_rt.clear();

			auto do_layer = [&](LayerData &l, bool left)
			{
				float angle = left ? M_PI / 2 : -M_PI / 2;
				int channel = left ? 0 : 1;
				auto &spectrum = left ? l.spectrum_left : l.spectrum_right;

				capture_time(
					"strided_copy",
					audioviz::util::strided_copy(
						l.a, audio_buffer.first(max_fft_size * num_channels), num_channels, channel));
				capture_time("fft", l.aa.execute_fft(l.fa, l.a));

				std::span<const float> amps;
				capture_time("compute_amps", amps = l.aa.compute_amplitudes(l.fa));

				capture_time(
					"resample_spectrum",
					audioviz::util::resample_spectrum(
						l.s, amps, sample_rate_hz, l.fa.get_fft_size(), 20.0f, 135.0f, ip));

				capture_time("spectrum_update", spectrum.update(l.s));
				audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, angle, M_PI);
				capture_time("draw", orig_rt.draw(spectrum, polar_rs));
			};

			// left channel
			for (auto &l : layers)
				capture_time("do_work", do_layer(l, true));

			// right channel
			for (auto &l : layers)
				capture_time("do_work", do_layer(l, false));

			orig_rt.display();
		});

	start_in_window(media, "polar-spectrum");
}

void OldBassNation::update(const std::span<const float> audio_buffer)
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
	OldBassNation viz{size, argv[3]};
}
