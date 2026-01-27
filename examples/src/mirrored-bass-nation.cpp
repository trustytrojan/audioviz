#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

#include <future>
#include <memory>
// #include <print>

#include "BassNationSpectrumLayer.hpp"

using namespace avz::examples;

struct MirroredBassNation : ExampleBase
{
	const int fft_size;

	std::vector<std::unique_ptr<BassNationSpectrumLayer>> spectrums;
	avz::ColorSettings cs;
	std::vector<std::future<void>> futures;

	// calculate the FFT amplitudes array indices of min/max frequencies in Hz
	const int min_fft_index = avz::util::bin_index_from_freq(0, sample_rate_hz, fft_size);
	const int max_fft_index = avz::util::bin_index_from_freq(250, sample_rate_hz, fft_size);
	std::vector<float> a, p;
	avz::ParticleSystem ps;
	avz::FrequencyAnalyzer fa{fft_size};
	avz::AudioAnalyzer aa;
	avz::fx::PolarCenter ps_polar{
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.x * 0.6f,		// Max radius: 60% screen width, so that you don't see any particles disappear
		M_PIf / 2,			// Angle start
		M_PI				// Angle span
	};

	avz::fx::Polar spectrum_polar{(sf::Vector2f)size, size.y * 0.25f, size.y * 0.5f, M_PI / 2, M_PI};
	avz::fx::Mirror mirror_l2r{0};

	MirroredBassNation(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, 75, config.framerate}
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
			spectrum.configure_spectrum(false, size);

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
			const auto amps = aa.get_amplitudes().subspan(min_fft_index, max_fft_index - min_fft_index);
			p.resize(amps.size());

			// weight each element by its normalized position through a Gompertz function
			// this makes lower bass frequencies boost the particles more than higher bass frequencies
			for (size_t i = 0; i < amps.size(); ++i)
			{
				const auto normalized_pos = (float)i / amps.size();
				p[i] = amps[i] * expf(-expf(4.5 * normalized_pos - 4)) / 5;
			}

			float max;
			capture_time("max", max = std::ranges::max(p));

			// pass the max as `additional_displacement` to ParticleSystem::update.
			// the displacement is then scaled to the window's height, and dampened
			// with sqrt (otherwise the particles just go crazy).
			capture_time("ps_update", ps.update(max));
		}

		// wait for all spectrums to finish updating
		std::ranges::for_each(futures, &std::future<void>::wait);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	MirroredBassNation, "Mirrored multi-spectrum polar visualization with bass frequencies", 0.25f, viz.fft_size)
