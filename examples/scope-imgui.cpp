#include <audioviz/Base.hpp>
#include <audioviz/ScopeDrawable.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <iostream>

struct ScopeImGuiTest : audioviz::Base
{
	audioviz::ColorSettings color;
	audioviz::ScopeDrawable<sf::RectangleShape> scope;

	ScopeImGuiTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

ScopeImGuiTest::ScopeImGuiTest(sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  scope{{{}, (sf::Vector2i)size}, color}
{
	scope.set_shape_width(5);
	scope.set_shape_spacing(2);

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	add_final_drawable(scope);

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window(media, "scope-imgui-test");
}

void ScopeImGuiTest::update(const std::span<const float> audio_buffer)
{
	ImGui::Begin("Scope Controls");
	scope.draw_imgui();
	ImGui::End();

	float left_channel[afpvf];
	// float right_channel[afpvf];
	for (int i = 0; i < afpvf; ++i)
	{
		const auto frame_idx = i * 2; // assuming stereo
		left_channel[i] = audio_buffer[frame_idx];
		// right_channel[i] = audio_buffer[frame_idx + 1];
	}

	scope.update({left_channel, (size_t)afpvf});
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	ScopeImGuiTest viz{size, argv[3]};
}
