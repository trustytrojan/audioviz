#include "avz/util.hpp"
#include <avz/fft/AudioAnalyzer.hpp>

#include <algorithm>

namespace avz
{

void AudioAnalyzer::execute_fft(FrequencyAnalyzer &fa, std::span<const float> audio)
{
	fa.copy_to_input(audio);
	fa.execute_fft();
	// reset state computed from previous fft output
	_amplitudes.clear();
}

void AudioAnalyzer::compute_amplitudes(const FrequencyAnalyzer &fa)
{
	const auto fft_size = fa.get_fft_size();
	const auto inv_fft_size = 1.f / fft_size;
	const auto fft_output_size = fft_size / 2 + 1;
	_amplitudes.resize(fft_output_size);

	auto *__restrict const out_ptr = _amplitudes.data();
	const auto *__restrict const in_ptr = fa.get_output().data();

#pragma GCC ivdep
	for (size_t i = 0; i < fft_output_size; ++i)
	{
		const auto re = in_ptr[i].real(), im = in_ptr[i].imag();
		// must divide by fft_size here to counteract the correlation
		// between fft_size and the average amplitude across the spectrum vector.
		out_ptr[i] = sqrtf((re * re) + (im * im)) * inv_fft_size;
	}
}

std::span<const float> AudioAnalyzer::get_amplitudes()
{
	if (_amplitudes.empty())
		throw std::logic_error{"[AudioAnalyzer::get_amplitudes] computed amplitudes required"};

	return _amplitudes;
}

AudioAnalyzer::FrequencyAmplitudePair
AudioAnalyzer::compute_peak_frequency(const FrequencyAnalyzer &fa, int sample_rate_hz, int from_hz, int to_hz)
{
	if (_amplitudes.empty())
		throw std::logic_error{"[AudioAnalyzer::compute_peak_frequency] computed amplitudes required"};

	assert(from_hz < to_hz);
	const auto fft_size = fa.get_fft_size();

	const auto start_bin = util::bin_index_from_freq(from_hz, sample_rate_hz, fft_size);
	const auto end_bin = util::bin_index_from_freq(to_hz, sample_rate_hz, fft_size);

	const auto amps_begin = _amplitudes.begin();
	const auto max_it = std::ranges::max_element(amps_begin + start_bin, amps_begin + end_bin + 1);
	const auto idx = std::distance(amps_begin, max_it);

	return {util::freq_from_bin_index(idx, sample_rate_hz, fft_size), *max_it};
}

} // namespace avz
