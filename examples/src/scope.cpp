#include "ExampleFramework.hpp"
#include <avz/ScopeDrawable.hpp>
#include <avz/util.hpp>

#include <vector>

using namespace avz::examples;

struct Scope : ExampleBase
{
	avz::ColorSettings color;
	avz::ScopeDrawable scope;
	std::vector<float> mono;
	const int required_frames;

	Scope(const ExampleConfig &config)
		: ExampleBase{config},
		  scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, color},
		  required_frames{std::max(1, (int)std::round(config.audio_duration_sec * sample_rate_hz))}
	{
		color.set_mode(avz::ColorSettings::Mode::SOLID);
		color.set_solid_color(sf::Color::Green);

		scope.set_audio_duration(config.audio_duration_sec);
		scope.set_sample_rate(sample_rate_hz);
		scope.set_shape_width(3);
		scope.set_shape_spacing(2);

		mono.resize(required_frames);

		auto &layer = emplace_layer<avz::Layer>("scope");
		layer.add_draw({scope});
	}

	void update(std::span<const float> audio_buffer) override
	{
		if (num_channels > 1)
		{
			avz::util::extract_channel(mono, audio_buffer, num_channels, 0);
			scope.update(mono);
		}
		else
		{
			std::span<const float> mono_span = audio_buffer.first(mono.size());
			scope.update(mono_span);
		}
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	Scope,
	"Audio oscilloscope visualization",
	0.02f,
	std::max(1, (int)std::round(config.audio_duration_sec *viz.sample_rate_hz)))
