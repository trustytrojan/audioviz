#include <audioviz/Base.hpp>
#include <audioviz/fft/AudioAnalyzer.hpp>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <audioviz/fx/Shake.hpp>
#include <audioviz/media/FfmpegPopenMedia.hpp>
#include <audioviz/util.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>
#include <numbers>

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

	// we want maximum frequency resolution to shake at precisely the right frequency!
	const int spectrum_size = fft_size / 2 + 1;

	audioviz::FrequencyAnalyzer fa;
	audioviz::AudioAnalyzer aa;
	float sample_rate_hz{};

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
			// orig_rt.draw(rect, &shake);
			orig_rt.clear();
			shake.apply(orig_rt, rect);
			orig_rt.display();
		});

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
	capture_time("fft", aa.analyze(fa, audio_buffer.data(), true, true));

	// Match ParticleSystem::update(const AudioAnalyzer&, UpdateOptions) behavior:
	// - average weighted bass max across channels
	// - scale by rect height
	// - apply displacement_func(scaled_avg / calm_factor)
	const float multiplier = 100.f;
	const float max_hz = 500.f;
	const float falloff_power = 0.f;

	float amp_sum = 0.f;
	float hz_sum = 0.f;

	for (int ch = 0; ch < aa.get_num_channels(); ++ch)
	{
		const auto &spec = aa.get_spectrum_data(ch);
		const auto idx = audioviz::util::weighted_max_index(spec, expf, 100.f);
		amp_sum += spec[idx];
		hz_sum += (static_cast<float>(idx) * sample_rate_hz) / static_cast<float>(fft_size);
	}

	const float amp_avg = amp_sum / aa.get_num_channels();
	const float hz_avg = hz_sum / aa.get_num_channels();

	const float amp = amp_avg * multiplier;
	shake.amplitude = {amp, amp};
	shake.frequency = 2.f * std::numbers::pi_v<float> * hz_avg; // convert Hz -> rad/s for sin()
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
