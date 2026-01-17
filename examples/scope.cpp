#include <audioviz/Base.hpp>
#include <audioviz/Player.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

constexpr float audio_duration_sec = 0.25f;

struct Scope : audioviz::Base
{
	audioviz::FfmpegPopenMedia media;
	int sample_rate_hz = media.audio_sample_rate();
	audioviz::ColorSettings color;
	audioviz::ScopeDrawable scope{{{10, 10}, {(int)size.x - 20, (int)size.y - 20}}, color};
	std::vector<float> mono;

	Scope(sf::Vector2u size, const std::string &media_url)
		: Base{size},
		  media{media_url}
	{
#ifdef __linux__
		enable_profiler();
		set_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

		color.set_mode(audioviz::ColorSettings::Mode::SOLID);
		color.set_solid_color(sf::Color::Green);

		scope.set_audio_duration(audio_duration_sec);
		scope.set_sample_rate(sample_rate_hz);
		scope.set_shape_width(3);
		scope.set_shape_spacing(2);

		const int required_frames = std::max(1, (int)std::round(audio_duration_sec * sample_rate_hz));
		mono.resize(required_frames);

		auto &layer = emplace_layer<audioviz::Layer>("scope");
		layer.add_draw({scope});
	}

	void update(std::span<const float> audio_buffer) override
	{
		if (media.audio_channels() > 1)
		{
			audioviz::util::extract_channel(mono, audio_buffer, media.audio_channels(), 0);
			scope.update(mono);
		}
		else
		{
			std::span<const float> mono_span = audio_buffer.first(mono.size());
			scope.update(mono_span);
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
	Scope viz{size, argv[3]};
	audioviz::Player{viz, viz.media, 60, (int)std::round(audio_duration_sec * viz.sample_rate_hz)}
		.start_in_window(argv[0]);
}
