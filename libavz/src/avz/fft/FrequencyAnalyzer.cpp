#include <avz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <cmath>

namespace audioviz
{

FrequencyAnalyzer::FrequencyAnalyzer(const int fft_size)
	: fft_size{fft_size}
{
	set_fft_size(fft_size);
}

void FrequencyAnalyzer::set_fft_size(const int fft_size)
{
	this->fft_size = fft_size;
	inv_fft_size = 1.0f / fft_size;
	fftw.set_n(fft_size);
	compute_window_values();
}

void FrequencyAnalyzer::set_window_func(const WindowFunction wf)
{
	window_func = wf;
	compute_window_values();
}

void FrequencyAnalyzer::copy_to_input(std::span<const float> wavedata)
{
	assert(wavedata.size() == fft_size);

	auto *__restrict const out_ptr = fftw.input().data();
	const auto *__restrict const in_ptr = wavedata.data();
	const auto *__restrict const win_ptr = window_values.data();

	if (window_func != WindowFunction::None)
#pragma GCC ivdep
		for (int i = 0; i < fft_size; ++i)
			out_ptr[i] = in_ptr[i] * win_ptr[i];
	else
#pragma GCC ivdep
		for (int i = 0; i < fft_size; ++i)
			out_ptr[i] = in_ptr[i];
}

void FrequencyAnalyzer::compute_amplitude(std::span<float> output) const
{
	const int size = output.size();
	assert(size == fftw.output_size());

	auto *__restrict const out_ptr = output.data();
	auto *__restrict const in_ptr = fftw.output().data();

#pragma GCC ivdep
	for (int i = 0; i < size; ++i)
	{
		const auto [re, im] = in_ptr[i];
		// must divide by fft_size here to counteract the correlation
		// between fft_size and the average amplitude across the spectrum vector.
		out_ptr[i] = sqrtf((re * re) + (im * im)) * inv_fft_size;
	}
}

void FrequencyAnalyzer::compute_phase(std::span<float> output) const
{
	const int size = output.size();
	assert(size == fftw.output_size());

	auto *__restrict const out_ptr = output.data();
	auto *__restrict const in_ptr = fftw.output().data();

#pragma GCC ivdep
	for (int i = 0; i < size; ++i)
	{
		const auto [re, im] = in_ptr[i];
		out_ptr[i] = atan2f(im, re);
	}
}

void FrequencyAnalyzer::compute_window_values()
{
	if (window_func == WindowFunction::None)
		return;

	window_values.resize(fft_size);

	if (window_func == WindowFunction::Hanning)
		apply_hanning_window();
	else if (window_func == WindowFunction::Blackman)
		apply_blackman_window();
	else if (window_func == WindowFunction::Hamming)
		apply_hamming_window();
}

void FrequencyAnalyzer::apply_hanning_window()
{
#pragma GCC ivdep
	for (int i = 0; i < fft_size; ++i)
		window_values[i] = 0.5f * (1 - cos(2 * M_PI * i / (fft_size - 1)));
}

void FrequencyAnalyzer::apply_hamming_window()
{
#pragma GCC ivdep
	for (int i = 0; i < fft_size; ++i)
		window_values[i] = 0.54f - 0.46f * cos(2 * M_PI * i / (fft_size - 1));
}

void FrequencyAnalyzer::apply_blackman_window()
{
#pragma GCC ivdep
	for (int i = 0; i < fft_size; ++i)
		window_values[i] =
			0.42f - 0.5f * cos(2 * M_PI * i / (fft_size - 1)) + 0.08f * cos(4 * M_PI * i / (fft_size - 1));
}

} // namespace audioviz
