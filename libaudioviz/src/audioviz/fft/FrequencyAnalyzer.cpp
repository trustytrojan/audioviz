#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "imgui.h"

namespace audioviz
{

void FrequencyAnalyzer::_scale_max::calc(const FrequencyAnalyzer &fa)
{
	const auto max = fa.fftw.output_size();
	linear = max;
	log = ::logf(max);
	sqrt = ::sqrtf(max);
	cbrt = ::cbrtf(max);
	nthroot = ::powf(max, fa.nthroot_inv);
}

FrequencyAnalyzer::FrequencyAnalyzer(const int fft_size)
	: fft_size{fft_size}
{
	set_fft_size(fft_size);
	m_spline_x.reserve(2048);
	m_spline_y.reserve(2048);
}

void FrequencyAnalyzer::set_fft_size(const int fft_size)
{
	this->fft_size = fft_size;
	inv_fft_size = 1.0f / fft_size;
	fftw.set_n(fft_size);
	scale_max.calc(*this);
	compute_index_ratios();
	compute_window_values();
}

void FrequencyAnalyzer::set_interp_type(const InterpolationType interp)
{
	this->interp = interp;
}

void FrequencyAnalyzer::set_window_func(const WindowFunction wf)
{
	this->window_func = wf;
	compute_window_values();
}

void FrequencyAnalyzer::set_accum_method(const AccumulationMethod am)
{
	this->am = am;
}

void FrequencyAnalyzer::set_scale(const Scale scale)
{
	this->scale = scale;
	compute_index_ratios();
}

void FrequencyAnalyzer::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument("FrequencySpectrum::set_nth_root: nth_root cannot be zero!");
	this->nth_root = nth_root;
	nthroot_inv = 1.f / nth_root;
	compute_index_ratios();
}

void FrequencyAnalyzer::copy_to_input(const float *const wavedata)
{
	const auto input = fftw.input();

	// this can be optimized by SIMD iff we convert from interleaved to planar beforehand...

	if (window_func)
		for (int i = 0; i < fft_size; ++i)
			input[i] = wavedata[i] * window_values[i];
	else
		memcpy(input, wavedata, fft_size * sizeof(float));
}

void FrequencyAnalyzer::copy_channel_to_input(
	const float *const audio, const int num_channels, const int channel, const bool interleaved)
{
	if (num_channels <= 0)
		throw std::invalid_argument("num_channels <= 0");
	if (channel < 0)
		throw std::invalid_argument("channel <= 0");
	if (channel >= num_channels)
		throw std::runtime_error("channel > num_channels");

	if (!interleaved)
	{
		copy_to_input(audio + (channel * fft_size));
		return;
	}

	// this can be optimized by SIMD iff we convert from interleaved to planar beforehand...

	const auto input = fftw.input();
	if (window_func)
		for (int i = 0; i < fft_size; ++i)
			input[i] = audio[i * num_channels + channel] * window_values[i];
	else
		for (int i = 0; i < fft_size; ++i)
			input[i] = audio[i * num_channels + channel];
}

void FrequencyAnalyzer::render(std::vector<float> &spectrum, bool skip_post_processing)
{
	const int size = spectrum.size();
	assert(size);

	if (size != known_spectrum_size)
	{
		known_spectrum_size = size;
		compute_index_ratios();
	}

	// execute fft and get output
	fftw.execute();

	if (skip_post_processing)
	{
		assert(size == fftw.output_size());
		for (int i = 0; i < size; ++i)
		{
			const float re = fftw.output()[i][0];
			const float im = fftw.output()[i][1];
			const float amplitude = (re * re) + (im * im);
			spectrum[i] = sqrtf(amplitude) * inv_fft_size;
		}
		return;
	}

	// zero out array since we are accumulating
	std::ranges::fill(spectrum, 0);

	// map frequency bins of freqdata to spectrum
	for (int i = 0; i < known_spectrum_size; ++i)
	{
		const auto [start, end] = spectrum_to_fftw_indices[i];

		if (start == -1)
			continue;

		float accumulated_amplitude{};
#pragma GCC ivdep
		for (int j = start; j < end; ++j)
		{
			// do NOT use destructuring of the fftwf_complex!
			// THIS allows the compiler to vectorize this entire loop body!
			const float re = fftw.output()[j][0];
			const float im = fftw.output()[j][1];
			const float amplitude = (re * re) + (im * im);
			switch (am)
			{
			case AccumulationMethod::MAX:
				accumulated_amplitude = std::max(accumulated_amplitude, amplitude);
				break;
			case AccumulationMethod::SUM:
				accumulated_amplitude += amplitude;
				break;
			}
		}

		// must divide by fft_size here to counteract the correlation
		// between fft_size and the average amplitude across the spectrum vector.
		spectrum[i] = sqrtf(accumulated_amplitude) * inv_fft_size;
	}

	// apply interpolation if necessary
	if (interp != InterpolationType::NONE && scale != Scale::LINEAR)
		interpolate(spectrum);
}

float FrequencyAnalyzer::calc_index_ratio(const float i) const
{
	switch (scale)
	{
	case Scale::LINEAR:
		return i / scale_max.linear;
	case Scale::LOG:
		return logf(i ? i : 1) / scale_max.log;
	case Scale::NTH_ROOT:
		switch (nth_root)
		{
		case 1:
			return i / scale_max.linear;
		case 2:
			return sqrtf(i) / scale_max.sqrt;
		case 3:
			return cbrtf(i) / scale_max.cbrt;
		default:
			return powf(i, nthroot_inv) / scale_max.nthroot;
		}
	default:
		throw std::logic_error("FrequencySpectrum::calc_index_ratio: default case hit");
	}
}

void FrequencyAnalyzer::compute_index_ratios()
{
	if (!known_spectrum_size)
		return;

	spectrum_to_fftw_indices.assign(known_spectrum_size, {-1, -1});

	fftw_to_spectrum_index.resize(fftw.output_size());
	for (int i = 0; i < fftw.output_size(); ++i)
	{
		const int spectrum_index = calc_index_ratio(i) * known_spectrum_size;
		fftw_to_spectrum_index[i] = std::clamp(spectrum_index, 0, known_spectrum_size - 1);
	}

	if (known_spectrum_size > 0)
	{
		for (int i = 0; i < fftw.output_size(); ++i)
		{
			const auto spectrum_index = fftw_to_spectrum_index[i];
			if (spectrum_to_fftw_indices[spectrum_index].first == -1)
				spectrum_to_fftw_indices[spectrum_index].first = i;
			spectrum_to_fftw_indices[spectrum_index].second = i + 1;
		}
	}
}

void FrequencyAnalyzer::compute_window_values()
{
	if (window_func)
	{
		window_values.resize(fft_size);
		for (int i = 0; i < fft_size; ++i)
			window_values[i] = window_func(i, fft_size);
	}
}

void FrequencyAnalyzer::interpolate(std::vector<float> &spectrum)
{
	const int size = spectrum.size();

	// separate the nonzero values (y's) and their indices (x's)
	m_spline_x.clear();
	m_spline_y.clear();
	for (int i = 0; i < size; ++i)
	{
		if (!spectrum[i])
			continue;
		m_spline_x.emplace_back(i);
		m_spline_y.emplace_back(spectrum[i]);
	}

	// tk::spline::set_points throws if there are less than 3 points
	if (m_spline_x.size() < 3)
		return;

	static const tk::spline::spline_type spline_type_table[] = {
		(tk::spline::spline_type)0,
		tk::spline::spline_type::linear,
		tk::spline::spline_type::cspline,
		tk::spline::spline_type::cspline_hermite};

	spline.set_points(m_spline_x, m_spline_y, spline_type_table[(int)interp]);

	// only copy spline values to fill in the gaps
	for (int i = 0; i < size; ++i)
		if (!spectrum[i])
			spectrum[i] = spline(i);
}

void FrequencyAnalyzer::draw_imgui()
{
	// FFT size (must be even). Keep user's intent when stepping: if they
	// decreased, move down to the next even; if they increased, move up.
	int fft_tmp = fft_size;
	if (ImGui::InputInt("FFT Size", &fft_tmp) && fft_tmp > 0)
		set_fft_size(fft_tmp);

	// Scale selection
	int scale_i = static_cast<int>(scale);
	if (ImGui::Combo("Scale", &scale_i, "Linear\0Log\0Nth Root\0"))
		set_scale(static_cast<Scale>(scale_i));
	if (static_cast<Scale>(scale_i) == Scale::NTH_ROOT)
	{
		int nth_tmp = nth_root;
		if (ImGui::SliderInt("Nth Root", &nth_tmp, 1, 32))
			set_nth_root(nth_tmp);
	}

	// Interpolation type
	int interp_i = static_cast<int>(interp);
	if (ImGui::Combo("Interpolation", &interp_i, "None\0Linear\0CSpline\0CSpline Hermite\0"))
		set_interp_type(static_cast<InterpolationType>(interp_i));

	// Accumulation method
	int am_i = static_cast<int>(am);
	if (ImGui::Combo("Accumulation", &am_i, "Sum\0Max\0"))
		set_accum_method(static_cast<AccumulationMethod>(am_i));

	// Window function selection
	static const WindowFunction *const wf_table[] = {{}, &WF_HANNING, &WF_HAMMING, &WF_BLACKMAN};
	if (ImGui::Combo("Window", &wf_i, "None\0Hanning\0Hamming\0Blackman\0"))
	{
		const auto wf = wf_table[wf_i];
		set_window_func(wf ? *wf : WindowFunction{});
	}
}

} // namespace audioviz
