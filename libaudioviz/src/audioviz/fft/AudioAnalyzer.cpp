#include <audioviz/fft/AudioAnalyzer.hpp>

#include <algorithm>
#include <stdexcept>

namespace audioviz
{

AudioAnalyzer::AudioAnalyzer(int sample_rate_hz, int fft_size)
	: sample_rate_hz{sample_rate_hz}
{
	set_fft_size(fft_size);
}

void AudioAnalyzer::execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio, bool interleaved)
{
	fa.copy_to_input(audio);
	fa.execute_fft();

	// reset state from computed previous fft data
	fft_amplitudes.clear();
	fft_phase.clear();
	peak_freq_amp.reset();
	multiband_shake.reset();
}

std::span<const float> AudioAnalyzer::compute_amplitudes(const FrequencyAnalyzer &fa)
{
	if (!fft_amplitudes.empty())
		return fft_amplitudes;
	fft_amplitudes.resize(fft_output_size);
	fa.compute_amplitude(fft_amplitudes);
	return fft_amplitudes;
}

std::span<const float> AudioAnalyzer::compute_phase(const FrequencyAnalyzer &fa)
{
	if (!fft_phase.empty())
		return fft_phase;
	fft_phase.resize(fft_output_size);
	fa.compute_phase(fft_phase);
	return fft_phase;
}

AudioAnalyzer::FrequencyAmplitudePair AudioAnalyzer::compute_peak_frequency(int from_hz, int to_hz)
{
	if (peak_freq_amp)
		return *peak_freq_amp;

	if (fft_amplitudes.empty())
		throw std::logic_error("compute_peak_frequency requires computed amplitudes");

	const auto to_bin = [&](int hz)
	{
		return std::clamp(hz * fft_size / sample_rate_hz, 0, static_cast<int>(fft_amplitudes.size()) - 1);
	};

	const int start_bin = to_bin(from_hz);
	const int end_bin = std::max(start_bin, to_bin(to_hz));

	const auto amps_begin = fft_amplitudes.begin();
	const auto max_it = std::ranges::max_element(amps_begin + start_bin, amps_begin + end_bin + 1);

	const int idx = std::distance(amps_begin, max_it);
	const float amplitude = *max_it;
	const float frequency_hz =
		static_cast<float>(idx) * static_cast<float>(sample_rate_hz) / static_cast<float>(fft_size);

	return *(peak_freq_amp = {frequency_hz, amplitude});
}

std::array<AudioAnalyzer::FrequencyAmplitudePair, 3>
AudioAnalyzer::compute_multiband_shake(int from_hz, int to_hz)
{
	if (multiband_shake)
		return *multiband_shake;

	if (fft_amplitudes.empty())
		throw std::logic_error("compute_multiband_shake requires computed amplitudes");

	const auto to_bin = [&](int hz)
	{
		return std::clamp(hz * fft_size / sample_rate_hz, 0, static_cast<int>(fft_amplitudes.size()) - 1);
	};

	const int start_bin = to_bin(from_hz);
	const int end_bin = std::max(start_bin, to_bin(to_hz));
	const int total_bins = end_bin - start_bin + 1;

	std::array<FrequencyAmplitudePair, 3> bands;

	// Divide the frequency range into 3 equal chunks
	const int chunk_size = std::max(1, total_bins / 3);

	const auto amps_begin = fft_amplitudes.begin();

	for (int i = 0; i < 3; i++)
	{
		const int chunk_start = start_bin + i * chunk_size;
		const int chunk_end = std::min(start_bin + (i + 1) * chunk_size, end_bin + 1);

		const auto max_it = std::ranges::max_element(amps_begin + chunk_start, amps_begin + chunk_end);
		const int idx = std::distance(amps_begin, max_it);

		bands[i].amplitude = *max_it;
		bands[i].frequency_hz = static_cast<int>(
			(static_cast<float>(idx) * static_cast<float>(sample_rate_hz)) / static_cast<float>(fft_size));
	}

	return *(multiband_shake = bands);
}

} // namespace audioviz
