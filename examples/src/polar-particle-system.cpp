#include "ExampleFramework.hpp"
#include <avz/analysis.hpp>
#include <avz/gfx.hpp>

using namespace avz::examples;

struct PolarParticleSystem : ExampleBase
{
	const int fft_size;

	// calculate the FFT amplitudes array indices of min/max frequencies in Hz
	const int min_fft_index = avz::util::bin_index_from_freq(0, sample_rate_hz, fft_size);
	const int max_fft_index = avz::util::bin_index_from_freq(250, sample_rate_hz, fft_size);

	// audio, spectrum
	std::vector<float> a, p;

	avz::ParticleSystem ps;
	avz::FrequencyAnalyzer fa;
	avz::AudioAnalyzer aa;

	avz::fx::PolarCenter polar{
		(sf::Vector2f)size, // Dimensions of linear space
		size.y * 0.25f,		// Base radius inner hole: 25% screen height
		size.x * 0.5f,		// Max radius: 50% screen width
		0,					// Angle start
		2 * M_PI			// Angle span
	};

	PolarParticleSystem(const ExampleConfig &config)
		: ExampleBase{config},
		  fft_size{static_cast<int>(config.audio_duration_sec * sample_rate_hz)},
		  ps{{{}, (sf::Vector2i)size}, 75, config.framerate},
		  fa{fft_size}
	{
		ps.set_fade_out(false);
		ps.set_start_offscreen(false);
		emplace_layer<avz::Layer>("particles").add_draw({ps, &polar});
	}

	void update(std::span<const float> audio_buffer) override
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
};

LIBAVZ_EXAMPLE_MAIN_CUSTOM(
	PolarParticleSystem,
	"Particle system with bass frequencies boosting particles in polar coordinates",
	0.25f,
	viz.fft_size)
