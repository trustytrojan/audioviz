#include <audioviz/Base.hpp>
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <future>
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
	// use one interpolator per layer to avoid sharing state across threads
	audioviz::Interpolator ip;

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

	void compute(std::span<const float> audio_buffer, int num_channels, int max_fft_size, int sample_rate_hz, bool left)
	{
		const int channel = left ? 0 : 1;
		auto &spectrum = left ? spectrum_left : spectrum_right;

		audioviz::util::strided_copy(a, audio_buffer.first(max_fft_size * num_channels), num_channels, channel);
		aa.execute_fft(fa, a);

		const auto amps = aa.compute_amplitudes(fa);

		audioviz::util::resample_spectrum(s, amps, sample_rate_hz, fa.get_fft_size(), 20.0f, 135.0f, ip);

		spectrum.update(s);
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

			const auto do_draw = [&](LayerData &l, bool left)
			{
				float angle = left ? M_PI / 2 : -M_PI / 2;
				auto &spectrum = left ? l.spectrum_left : l.spectrum_right;
				audioviz::fx::Polar::setParameters((sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, angle, M_PI);
				orig_rt.draw(spectrum, polar_rs);
			};

			std::vector<std::future<void>> futures;
			futures.reserve(layers.size() * 2);

			for (auto &l : layers)
			{
				futures.push_back(
					std::async(
						std::launch::async,
						&LayerData::compute,
						&l,
						audio_buffer,
						num_channels,
						max_fft_size,
						sample_rate_hz,
						true));
			}

			// wait for all compute tasks
			for (auto &f : futures)
				f.wait();

			for (auto &l : layers)
			{
				futures.push_back(
					std::async(
						std::launch::async,
						&LayerData::compute,
						&l,
						audio_buffer,
						num_channels,
						max_fft_size,
						sample_rate_hz,
						false));
			}

			// wait for all compute tasks
			for (auto &f : futures)
				f.wait();

			// left channel
			std::ranges::for_each(layers, [&](auto &l) { do_draw(l, true); });

			// right channel
			std::ranges::for_each(layers, [&](auto &l) { do_draw(l, false); });

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
