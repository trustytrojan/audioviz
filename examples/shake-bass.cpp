#include "ExampleFramework.hpp"
#include <avz/fft/FrequencyAnalyzer.hpp>
#include <avz/fft/StereoAnalyzer.hpp>
#include <avz/fx/Shake.hpp>

#include <SFML/Graphics.hpp>

using namespace audioviz::examples;

struct ShakeBassTest : ExampleBase<ShakeBassTest>
{
	const int fft_size;

	audioviz::FrequencyAnalyzer fa{fft_size};
	audioviz::StereoAnalyzer sa;

	sf::RectangleShape rect;
	audioviz::fx::Shake shake;

	ShakeBassTest(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{config.audio_duration_sec * sample_rate_hz},
		  sa{static_cast<float>(sample_rate_hz), fft_size},
		  rect{sf::Vector2f{size.x * 0.45f, size.y * 0.45f}}
	{
		rect.setOrigin(rect.getGeometricCenter());
		rect.setPosition(sf::Vector2f{size} * 0.5f);
		rect.setFillColor(sf::Color::Transparent);
		rect.setOutlineColor(sf::Color::White);
		rect.setOutlineThickness(1);

		emplace_layer<audioviz::Layer>("shake").add_draw({rect, &shake});
	}

	void update(std::span<const float> audio_buffer) override
	{
		capture_time("fft", sa.execute_fft(fa, audio_buffer));
		sa.left().compute_amplitudes(fa);
		shake.setParameters(sa.left(), 0, 250, 100.f);
	}
};

AUDIOVIZ_EXAMPLE_MAIN_CUSTOM(ShakeBassTest, "Shake effect visualization based on bass frequencies", viz.fft_size)
