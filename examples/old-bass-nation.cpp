#include "ExampleFramework.hpp"
#include <audioviz/SpectrumDrawable.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fft/Interpolator.hpp>
#include <audioviz/fx/Polar.hpp>
#include <audioviz/util.hpp>

#include <future>
#include <memory>
// #include <print>

#include "SpectrumLayer.hpp"

using namespace audioviz::examples;

struct OldBassNation : ExampleBase<OldBassNation>
{
	const int max_fft_size;

	std::vector<std::unique_ptr<SpectrumLayer>> spectrums;
	audioviz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	audioviz::fx::Polar polar_left;
	audioviz::fx::Polar polar_right;

	OldBassNation(const ExampleConfig &config)
		: ExampleBase{config},
		  max_fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  polar_left{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI},
		  polar_right{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, -M_PI / 2, M_PI}
	{
		// std::println("max_fft_size={} sample_rate_hz={}", max_fft_size, sample_rate_hz);

		cs.set_mode(audioviz::ColorSettings::Mode::SOLID);

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

		auto &spectrum_layer = emplace_layer<audioviz::PostProcessLayer>("spectrum", size);

		for (int i = 0; i < colors.size(); ++i)
		{
			float duration_diff = max_duration_diff - i * delta_duration;
			const auto new_duration_sec = config.audio_duration_sec - duration_diff;
			const int new_fft_size = new_duration_sec * sample_rate_hz;

			cs.set_solid_color(colors[i]);

			auto &left_layer =
				*spectrums.emplace_back(std::make_unique<SpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, true));
			left_layer.configure_spectrum(false, size);

			auto &right_layer =
				*spectrums.emplace_back(std::make_unique<SpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, false));
			right_layer.configure_spectrum(true, size);

			spectrum_layer.add_draw({left_layer.spectrum, &polar_left});
			spectrum_layer.add_draw({right_layer.spectrum, &polar_right});
		}

		futures.resize(spectrums.size());
	}

	void update(std::span<const float> audio_buffer) override
	{
		// std::ranges::transform(spectrums, futures.begin(), [=](auto &l) { return l->trigger_work(audio_buffer); });
		std::ranges::transform(spectrums, futures.begin(), std::bind_back(&SpectrumLayer::trigger_work, audio_buffer));

		// wait for all compute tasks
		std::ranges::for_each(futures, &std::future<void>::wait);
	}
};

AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(
	OldBassNation, "Multi-spectrum polar visualization with bass frequencies (old version)", viz.max_fft_size)
