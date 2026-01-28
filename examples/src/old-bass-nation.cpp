#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#include <future>
#include <memory>
// #include <print>

#include "BassNationSpectrumLayer.hpp"

using namespace avz::examples;

struct OldBassNation : ExampleBase
{
	const int max_fft_size;

	std::vector<std::unique_ptr<BassNationSpectrumLayer>> spectrums;
	avz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	avz::fx::Polar polar_left;
	avz::fx::Polar polar_right;

	OldBassNation(const ExampleConfig &config)
		: ExampleBase{config},
		  max_fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  polar_left{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI},
		  polar_right{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, -M_PI / 2, M_PI}
	{
		cs.set_mode(avz::ColorSettings::Mode::SOLID);

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

		auto &spectrum_layer = emplace_layer<avz::Layer>("spectrum");

		for (int i = 0; i < colors.size(); ++i)
		{
			float duration_diff = max_duration_diff - i * delta_duration;
			const auto new_duration_sec = config.audio_duration_sec - duration_diff;
			const int new_fft_size = new_duration_sec * sample_rate_hz;

			cs.set_solid_color(colors[i]);

			auto &left_layer = *spectrums.emplace_back(
				std::make_unique<BassNationSpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, true));
			left_layer.configure_spectrum(false, size);

			auto &right_layer = *spectrums.emplace_back(
				std::make_unique<BassNationSpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, false));
			right_layer.configure_spectrum(true, size);

			spectrum_layer.add_draw({left_layer.spectrum, &polar_left});
			spectrum_layer.add_draw({right_layer.spectrum, &polar_right});
		}

		futures.resize(spectrums.size());
	}

	void update(std::span<const float> audio_buffer) override
	{
		// std::ranges::transform(spectrums, futures.begin(), [=](auto &l) { return l->trigger_work(audio_buffer); });
		std::ranges::transform(
			spectrums, futures.begin(), std::bind_back(&BassNationSpectrumLayer::trigger_work, audio_buffer));

		// wait for all compute tasks
		std::ranges::for_each(futures, &std::future<void>::wait);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	OldBassNation, "Multi-spectrum polar visualization with bass frequencies (old version)", 0.25f, viz.max_fft_size)
