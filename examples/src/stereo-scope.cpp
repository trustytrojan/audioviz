// Use two ScopeDrawables directly instead of StereoScope
#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#include <algorithm>
#include <cmath>
#include <print>
#include <vector>

using namespace avz::examples;

struct StereoScopeViz : ExampleBase
{
	avz::ColorSettings colorL, colorR;
	avz::ScopeDrawable left_scope;
	avz::ScopeDrawable right_scope;
	std::vector<float> left_channel, right_channel;
	const int required_frames;

	StereoScopeViz(const ExampleConfig &config)
		: ExampleBase{config},
		  left_scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, colorL},
		  right_scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, colorR},
		  required_frames{config.audio_duration_sec * sample_rate_hz}
	{
		std::println("required_frames={}", required_frames);

		colorL.set_mode(avz::ColorSettings::Mode::SOLID);
		colorL.set_solid_color(sf::Color::Red);
		colorR.set_mode(avz::ColorSettings::Mode::SOLID);
		colorR.set_solid_color(sf::Color::Cyan);

		left_scope.set_audio_duration(config.audio_duration_sec);
		left_scope.set_sample_rate(sample_rate_hz);
		left_scope.set_shape_width(10);
		left_scope.set_shape_spacing(5);

		right_scope.set_audio_duration(config.audio_duration_sec);
		right_scope.set_sample_rate(sample_rate_hz);
		right_scope.set_shape_width(10);
		right_scope.set_shape_spacing(5);
		right_scope.set_backwards(true);

		left_channel.resize(required_frames);
		right_channel.resize(required_frames);

		auto &layer = emplace_layer<avz::Layer>("stereo-scope");
		layer.add_draw({left_scope});
		layer.add_draw({right_scope});
	}

	void update(std::span<const float> audio_buffer) override
	{
		if (num_channels > 1)
		{
			std::println(
				"left_channel.size()={} audio_buffer.size()={} num_channels={}",
				left_channel.size(),
				audio_buffer.size(),
				num_channels);
			avz::util::extract_channel(left_channel, audio_buffer, num_channels, 0);
			avz::util::extract_channel(right_channel, audio_buffer, num_channels, 1);
			left_scope.update(left_channel);
			right_scope.update(right_channel);
		}
		else
		{
			std::span<const float> mono_span = audio_buffer.first(left_channel.size());
			left_scope.update(mono_span);
			right_scope.update(mono_span);
		}
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	StereoScopeViz,
	"Stereo oscilloscope visualization with left (red) and right (cyan) channels",
	0.02f,
	viz.required_frames)
