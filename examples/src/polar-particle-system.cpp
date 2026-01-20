#include "ExampleFramework.hpp"
#include <avz/ParticleSystem.hpp>
#include <avz/aligned_allocator.hpp>
#include <avz/fft/AudioAnalyzer.hpp>
#include <avz/fft/FrequencyAnalyzer.hpp>
#include <avz/fx/Polar.hpp>
#include <avz/util.hpp>

/*
TODO: fix the ExampleFramework to accept default fft sizes from programs
TODO: make new polar shader without coordinate warping
*/

using namespace avz::examples;

struct PolarParticleSysten : ExampleBase<PolarParticleSysten>
{
	const int fft_size;

	// audio, spectrum
	std::vector<float, aligned_allocator<float>> a, s;

	avz::ParticleSystem<sf::CircleShape> ps;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	avz::fx::Polar polar{
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.x * 0.5f,		// Max radius: 50% screen width
		0,					// Angle start
		2 * M_PI			// Angle span
	};

	PolarParticleSysten(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, 100},
		  fa{fft_size},
		  aa{sample_rate_hz, fft_size}
	{
		emplace_layer<avz::Layer>("particles").add_draw({ps, &polar});
	}

	void update(std::span<const float> audio_buffer) override
	{
		// make sure we can fit one channel of audio
		a.resize(fft_size);

		// extract first channel of audio and perform FFT (needed in case media file has stereo audio)
		capture_time("strided_copy", avz::util::extract_channel(a, audio_buffer, num_channels, 0));
		capture_time("fft", aa.execute_fft(fa, a));

		// compute_peak_frequency() requires already computed FFT amplitudes in the analyzer
		aa.compute_amplitudes(fa);

		// let the bass boost the particles' velocities
		// lower bass has more power than higher bass
		const auto a1 = aa.compute_peak_frequency(0, 125).amplitude;
		const auto a2 = aa.compute_peak_frequency(126, 250).amplitude / 2;
		ps.update((a1 + a2) / 2);
	}
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	PolarParticleSysten, "Particle system with bass frequencies boosting particles in polar coordinates", viz.fft_size)
