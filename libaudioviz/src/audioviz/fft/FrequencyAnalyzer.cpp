#include <algorithm>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <cmath>
#include <print>

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
	std::println("fftw output size: {}", fftw.output_size());
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

	// this can be optimized by SIMD if we use a planar format...
	const auto fftw_input = fftw.input();
	if (window_func)
		for (int i = 0; i < fft_size; ++i)
			fftw_input[i] = wavedata[i] * window_values[i];
	else
		std::ranges::copy(wavedata, fftw_input);
}

void FrequencyAnalyzer::copy_channel_to_input(
	std::span<const float> audio, const int num_channels, const int channel, const bool interleaved)
{
	assert(num_channels > 0);
	assert(audio.size() >= fft_size * num_channels);
	assert(channel >= 0);
	assert(channel < num_channels);

	if (!interleaved)
	{
		copy_to_input(audio.subspan(channel * fft_size));
		return;
	}

	// this can be optimized by SIMD if we use a planar format...
	const auto fftw_input = fftw.input();
	if (window_func)
		for (int i = 0; i < fft_size; ++i)
			fftw_input[i] = audio[i * num_channels + channel] * window_values[i];
	else
		for (int i = 0; i < fft_size; ++i)
			fftw_input[i] = audio[i * num_channels + channel];
}

void FrequencyAnalyzer::compute_amplitude(std::span<float> output)
{
	const int size = output.size();
	assert(size == fftw.output_size());
	for (int i = 0; i < size; ++i)
	{
		const float re = fftw.output()[i][0];
		const float im = fftw.output()[i][1];
		// must divide by fft_size here to counteract the correlation
		// between fft_size and the average amplitude across the spectrum vector.
		output[i] = sqrtf((re * re) + (im * im)) * inv_fft_size;
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
