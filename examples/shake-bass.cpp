#include <audioviz/Base.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fx/Shake.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <cmath>
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
	const int spectrum_size = 512;

	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;

	sf::RectangleShape rect;
	audioviz::fx::Shake shake;

	ShakeBassTest(sf::Vector2u size, const std::string &media_url);
	void update(std::span<const float> audio_buffer) override;
};

ShakeBassTest::ShakeBassTest(const sf::Vector2u size, const std::string &media_url)
	: Base{size},
	  fa{fft_size},
	  aa{2},
	  rect{sf::Vector2f{size.x * 0.45f, size.y * 0.45f}},
	  shake{{0.f, 0.f}, 25.f}
{
	aa.resize(spectrum_size);
	set_audio_frames_needed(fft_size);

	rect.setOrigin(rect.getGeometricCenter());
	rect.setPosition(sf::Vector2f{size} * 0.5f);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color::White);
	rect.setOutlineThickness(1);

	// One layer, one drawable, one effect.
	auto &layer = add_layer("shake");
	// layer.add_drawable(&rect);
	// layer.add_effect(&shake);
	layer.set_orig_cb(
		[&](auto &orig_rt)
		{
			orig_rt.draw(rect, &shake);
			orig_rt.display();
		});

#ifdef __linux__
	set_timing_text_enabled(true);
	set_text_font("/usr/share/fonts/TTF/Iosevka-Regular.ttc");
#endif

	audioviz::FfmpegPopenMedia media{media_url, size};
	start_in_window(media, "shake-bass");
}

void ShakeBassTest::update(const std::span<const float> audio_buffer)
{
	capture_time("fft", aa.analyze(fa, audio_buffer.data(), true));

	// Match ParticleSystem::update(const AudioAnalyzer&, UpdateOptions) behavior:
	// - average weighted bass max across channels
	// - scale by rect height
	// - apply displacement_func(scaled_avg / calm_factor)
	float avg{};
	for (int i = 0; i < aa.get_num_channels(); ++i)
		avg += audioviz::util::weighted_max(aa.get_spectrum_data(i), sqrtf);
	avg /= aa.get_num_channels();

	const float calm_factor = 5.f;
	const float multiplier = 10.f;
	const auto scaled_avg = static_cast<float>(rect.getSize().y) * avg;
	const auto additional_displacement = sqrtf(scaled_avg / calm_factor);
	const auto amp = additional_displacement * multiplier;

	shake.amplitude = {amp, amp};
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
