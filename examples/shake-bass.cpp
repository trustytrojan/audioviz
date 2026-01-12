#include <audioviz/Base.hpp>
#include <audioviz/fft/StereoAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fx/Shake.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

#define capture_time(label, code)            \
	if (timing_text_enabled())               \
	{                                        \
		sf::Clock _clock;                    \
		code;                                \
		capture_elapsed_time(label, _clock); \
	}                                        \
	else                                     \
		code;

struct ShakeBassTest : audioviz::Base
{
	const int fft_size = 3000;

	audioviz::FrequencyAnalyzer fa;
	audioviz::StereoAnalyzer sa;
	float sample_rate_hz{};

	sf::RectangleShape rect;

	ShakeBassTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

ShakeBassTest::ShakeBassTest(const sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  fa{fft_size},
	  rect{sf::Vector2f{size.x * 0.45f, size.y * 0.45f}}
{
	set_audio_frames_needed(fft_size);

	rect.setOrigin(rect.getGeometricCenter());
	rect.setPosition(sf::Vector2f{size} * 0.5f);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color::White);
	rect.setOutlineThickness(1);

	// auto &layer = add_layer("shake");
	// layer.add_draw({rect, &audioviz::fx::Shake::getShader()});
	add_final_drawable2(rect, &audioviz::fx::Shake::getShader());

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	audioviz::FfmpegPopenMedia media{media_url, size};
	sample_rate_hz = static_cast<float>(media.audio_sample_rate());
	start_in_window(media, "shake-bass");
}

void ShakeBassTest::update(const std::span<const float> audio_buffer)
{
	capture_time("fft", sa.execute_fft(fa, audio_buffer, true));
	audioviz::fx::Shake::setParameters(sa, sample_rate_hz, fft_size, 100.f);
}

int main(const int argc, const char *const *const argv)
{
	if (argc < 4)
	{
		std::cerr << "usage: " << argv[0] << " <size.x> <size.y> <media file>\n";
		return EXIT_FAILURE;
	}

	const sf::Vector2u size{std::stoul(argv[1]), std::stoul(argv[2])};
	ShakeBassTest viz{size, argv[3]};
}
