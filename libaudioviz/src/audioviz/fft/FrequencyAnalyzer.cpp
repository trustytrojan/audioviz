#include <algorithm>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <cmath>
#include <memory>

#include "imgui.h"

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
	this->window_func = wf;
	compute_window_values();
}

void FrequencyAnalyzer::copy_to_input(std::span<const float> wavedata)
{
	assert(wavedata.size() == fft_size);

	auto *__restrict const out_ptr = std::assume_aligned<32>(fftw.input().data());
	const auto *__restrict const in_ptr = std::assume_aligned<32>(wavedata.data());
	const auto *__restrict const win_ptr = std::assume_aligned<32>(window_values.data());

	if (window_func)
#pragma GCC ivdep
		for (int i = 0; i < fft_size; ++i)
			out_ptr[i] = in_ptr[i] * win_ptr[i];
	else
		std::ranges::copy(wavedata, out_ptr);
}

void FrequencyAnalyzer::compute_amplitude(std::span<float> output) const
{
	const int size = output.size();
	assert(size == fftw.output_size());

	auto *__restrict const out_ptr = std::assume_aligned<32>(output.data());
	auto *__restrict const in_ptr = std::assume_aligned<32>(fftw.output().data());

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
	for (int i = 0; i < size; ++i)
	{
		const auto [re, im] = fftw.output()[i];
		output[i] = atan2f(im, re);
	}
}

void FrequencyAnalyzer::compute_window_values()
{
	if (!window_func)
		return;
	window_values.resize(fft_size);
	for (int i = 0; i < fft_size; ++i)
		window_values[i] = window_func(i, fft_size);
}

void FrequencyAnalyzer::draw_imgui()
{
	// FFT size (must be even). Keep user's intent when stepping: if they
	// decreased, move down to the next even; if they increased, move up.
	int fft_tmp = fft_size;
	if (ImGui::InputInt("FFT Size", &fft_tmp) && fft_tmp > 0)
		set_fft_size(fft_tmp);

	// Window function selection
	static const WindowFunction *const wf_table[] = {{}, &WF_HANNING, &WF_HAMMING, &WF_BLACKMAN};
	if (ImGui::Combo("Window", &wf_i, "None\0Hanning\0Hamming\0Blackman\0"))
	{
		const auto wf = wf_table[wf_i];
		set_window_func(wf ? *wf : WindowFunction{});
	}
}

} // namespace audioviz
