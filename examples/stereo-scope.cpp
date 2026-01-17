// Use two ScopeDrawables directly instead of StereoScope
#include <audioviz/Base.hpp>
#include <audioviz/Player.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

constexpr float audio_duration_sec = 0.05f;

struct StereoScopeViz : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	int num_channels = media.audio_channels();
	audioviz::ColorSettings colorL, colorR;
	audioviz::ScopeDrawable left_scope;
	audioviz::ScopeDrawable right_scope;
	std::vector<float> left_channel, right_channel;

	StereoScopeViz(sf::Vector2u size, const std::string &media_url)
		: Base{size},
		  media{media_url},
		  colorL{},
		  colorR{},
		  left_scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, colorL},
		  right_scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, colorR}
	{
#ifdef __linux__
		enable_profiler();
		set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

		colorL.set_mode(audioviz::ColorSettings::Mode::SOLID);
		colorL.set_solid_color(sf::Color::Red);
		colorR.set_mode(audioviz::ColorSettings::Mode::SOLID);
		colorR.set_solid_color(sf::Color::Cyan);

		left_scope.set_audio_duration(audio_duration_sec);
		left_scope.set_sample_rate(sample_rate_hz);
		left_scope.set_shape_width(10);
		left_scope.set_shape_spacing(5);

		right_scope.set_audio_duration(audio_duration_sec);
		right_scope.set_sample_rate(sample_rate_hz);
		right_scope.set_shape_width(10);
		right_scope.set_shape_spacing(5);
		right_scope.set_backwards(true);

		const int required_frames = std::max(1, (int)std::round(audio_duration_sec * sample_rate_hz));
		left_channel.resize(required_frames);
		right_channel.resize(required_frames);

		auto &layer = emplace_layer<audioviz::Layer>("stereo-scope");
		layer.add_draw({left_scope});
		layer.add_draw({right_scope});
	}

	void update(std::span<const float> audio_buffer) override
	{
		if (num_channels > 1)
		{
			audioviz::util::extract_channel(left_channel, audio_buffer, num_channels, 0);
			audioviz::util::extract_channel(right_channel, audio_buffer, num_channels, 1);
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

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	StereoScopeViz viz{size, argv[3]};
	audioviz::Player{viz, viz.media, 60, (int)std::round(audio_duration_sec * viz.sample_rate_hz)}
		.start_in_window(argv[0]);
}
