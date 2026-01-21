#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/fx/Shake.hpp>

#include <SFML/Graphics.hpp>

using namespace avz::examples;

struct ShakeBassTest : ExampleBase
{
	const int fft_size;

	avz::FrequencyAnalyzer fa{fft_size};
	avz::StereoAnalyzer sa;

	sf::RectangleShape rect;
	avz::fx::Shake shake;

	ShakeBassTest(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{config.audio_duration_sec * sample_rate_hz},
		  rect{sf::Vector2f{size.x * 0.45f, size.y * 0.45f}}
	{
		rect.setOrigin(rect.getGeometricCenter());
		rect.setPosition(sf::Vector2f{size} * 0.5f);
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1);

		emplace_layer<avz::Layer>("shake").add_draw({rect, &shake});
	}

	void update(std::span<const float> audio_buffer) override
	{
		capture_time("fft", sa.execute_fft(fa, audio_buffer));

		// amplitudes are required to be computed before calling compute_peak_frequency()
		sa.compute_amplitudes(fa);

		// split up 0-250Hz into three bands: low bass, mid bass, high bass.
		// since we are calling this on a StereoAnalyzer, it will average the peak frequency
		// of the frequency range across both channels.
		constexpr auto band_size = 250 / 3;
		const auto [f1, a1] = sa.compute_averaged_peak_frequency(fa, sample_rate_hz, 0, band_size);
		const auto [f2, a2] = sa.compute_averaged_peak_frequency(fa, sample_rate_hz, band_size + 1, 2 * band_size);
		const auto [f3, a3] = sa.compute_averaged_peak_frequency(fa, sample_rate_hz, 2 * band_size + 1, 3 * band_size);

		// pass the three frequencies & amplitudes to Shake effect
		shake.setParameters({f1, f2, f3}, {a1, a2, a3}, 100);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(ShakeBassTest, "Shake effect visualization based on bass frequencies", 0.25f, viz.fft_size)
