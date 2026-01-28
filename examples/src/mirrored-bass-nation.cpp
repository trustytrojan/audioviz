#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#include <future>
#include <memory>
// #include <print>

#include "BassNationSpectrumLayer.hpp"

using namespace avz::examples;

/**
 * Calculates the additional particle displacement using the bass frequency spectrum passed
 * as `bass_amps`. It is expected to contain a 20-135Hz frequency range, straight from FFT
 * amplitude output.
 */
float bass_nation_additional_displacement(std::span<const float> bass_amps)
{
	// Constants for the "Nation" look
	constexpr auto mid_point = 0.3f;	// Full weight at 53Hz
	constexpr auto k_rise = 0.08f;		// Smooth, quick rise from 20Hz
	constexpr auto k_fall = 0.08f;		// Gentle fall toward 135Hz
	constexpr auto threshold = 1.0f;	// Your "Kick" detection threshold
	constexpr auto boost_factor = 3.0f; // How much the "Bonus" multiplies

	float max{};

	for (size_t i = 0; i < bass_amps.size(); ++i)
	{
		const auto x = (float)i / bass_amps.size();

		// 1. Optimized Asymmetric Weighting
		const auto dist = x - mid_point;
		const auto k = (dist < 0) ? k_rise : k_fall;
		const auto freq_weight = std::exp(-(dist * dist) / k);

		// 2. Apply weight to raw amplitude
		auto val = bass_amps[i] * freq_weight;

		// 3. The "Kick Bonus" (Exponential Expansion)
		// Example: if val is 1.2, the 'extra' 0.2 is taken out, squared, then added back to val.
		if (val > threshold)
		{
			const auto extra = val - threshold;
			val = threshold + (extra * extra * boost_factor);
		}

		// Dividing by 5, for some reason, makes it look perfect.
		val /= 5;

		// Keep finding the max.
		if (val > max)
			max = val;
	}

	return max;
}

struct MirroredBassNation : ExampleBase
{
	const int fft_size;

	std::vector<std::unique_ptr<BassNationSpectrumLayer>> spectrums;
	avz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	// calculate the FFT amplitudes array indices of min/max frequencies in Hz
	const int min_fft_index = avz::util::bin_index_from_freq(20, sample_rate_hz, fft_size);
	const int max_fft_index = avz::util::bin_index_from_freq(135, sample_rate_hz, fft_size);
	std::vector<float> a, p;
	avz::ParticleSystem ps;
	avz::FrequencyAnalyzer fa{fft_size};
	avz::AudioAnalyzer aa;
	avz::fx::Polar ps_polar{
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.x * 0.6f,		// Max radius: 60% screen width, so that you don't see any particles disappear
		M_PI / 2,			// Angle start
		M_PI,				// Angle span
		0.0f				// warping_factor: no warping for particles
	};

	avz::fx::Polar spectrum_polar{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI};
	avz::fx::Mirror mirror_l2r{0};

	MirroredBassNation(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, 50, config.framerate}
	{
		ps.set_fade_out(false);
		ps.set_start_offscreen(false);
		auto &particles_layer = emplace_layer<avz::PostProcessLayer>("particles", size);
		particles_layer.add_draw({ps, &ps_polar});
		particles_layer.add_effect(&mirror_l2r);

		cs.set_mode(avz::ColorSettings::Mode::SOLID);

		static const std::array<sf::Color, 9> colors{
			sf::Color::Green,
			sf::Color::Cyan,
			sf::Color::Blue,
			sf::Color{146, 29, 255}, // purple
			sf::Color::Magenta,
			sf::Color::Red,
			sf::Color{255, 165, 0}, // orange
			sf::Color::Yellow,
			sf::Color::White //
		};

		const auto delta_duration = 0.015f;
		const auto max_duration_diff = (colors.size() - 1) * delta_duration;

		auto &spectrum_layer = emplace_layer<avz::PostProcessLayer>("spectrum", size);

		for (int i = 0; i < colors.size(); ++i)
		{
			float duration_diff = max_duration_diff - i * delta_duration;
			const auto new_duration_sec = config.audio_duration_sec - duration_diff;
			const int new_fft_size = new_duration_sec * sample_rate_hz;

			cs.set_solid_color(colors[i]);

			auto &spectrum = *spectrums.emplace_back(
				std::make_unique<BassNationSpectrumLayer>(new_fft_size, sample_rate_hz, size, cs, true));
			spectrum.spectrum.set_use_gs(true);
			spectrum.configure_spectrum(false, size);

			// Enable GS expansion on the polar effect
			spectrum_polar.set_gs_expansion(true, spectrum.spectrum.get_bar_width(),
											spectrum.spectrum.get_rect().position.y +
												spectrum.spectrum.get_rect().size.y);

			spectrum_layer.add_draw({spectrum.spectrum, &spectrum_polar});
		}

		// Add mirror effect to create mirrored versions on the right side
		spectrum_layer.add_effect(&mirror_l2r);

		futures.resize(spectrums.size());
	}

	void update(std::span<const float> audio_buffer) override
	{
		// start an update for each spectrum
		std::ranges::transform(
			spectrums, futures.begin(), std::bind_back(&BassNationSpectrumLayer::trigger_work, audio_buffer));

		// while the spectrums are updating, update our particle system
		{
			// make sure we can fit one channel of audio
			a.resize(fft_size);

			// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
			capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
			capture_time("fft", aa.execute_fft(fa, a));

			// compute amplitudes from FFT output
			capture_time("amplitudes", aa.compute_amplitudes(fa));

			// compute FFT amplitudes, take only the bass frequencies
			const auto bass_amps = aa.get_amplitudes().subspan(min_fft_index, max_fft_index - min_fft_index);
			p.resize(bass_amps.size());

			// calculate and pass additional particle displacement to the ParticleSystem's update() method
			capture_time("ps_update", ps.update(bass_nation_additional_displacement(bass_amps)));
		}

		// wait for all spectrums to finish updating
		std::ranges::for_each(futures, &std::future<void>::wait);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	MirroredBassNation, "Mirrored multi-spectrum polar visualization with bass frequencies", 0.25f, viz.fft_size)
