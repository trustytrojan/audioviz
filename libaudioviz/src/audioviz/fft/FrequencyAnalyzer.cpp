#include <algorithm>
#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cassert>
#include <cmath>
#include <print>
#include <stdexcept>

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
	compute_index_mappings();
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
	compute_index_mappings();
}

void FrequencyAnalyzer::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument("FrequencySpectrum::set_nth_root: nth_root cannot be zero!");
	this->nth_root = nth_root;
	nthroot_inv = 1.f / nth_root;
	compute_index_mappings();
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

void FrequencyAnalyzer::execute_fft(std::span<float> output)
{
	fftw.execute();
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

void FrequencyAnalyzer::bin_pack(std::span<float> out, std::span<const float> in)
{
	assert(in.size() == fftw.output_size());
	const int size = out.size();

	if (size != known_spectrum_size)
	{
		known_spectrum_size = size;
		compute_index_mappings();
	}

	// just keep doing it here for consistency
	std::ranges::fill(out, 0);

	for (int i = 0; i < known_spectrum_size; ++i)
	{
		const auto [start, end] = spectrum_to_fftw_indices[i];

		if (start == -1)
			continue;

		float accumulated_amplitude{};
		for (int j = start; j < end; ++j)
		{
			const float amplitude = in[j];
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

		out[i] = accumulated_amplitude;
	}
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

void FrequencyAnalyzer::compute_index_mappings()
{
	if (!known_spectrum_size)
		return;

	spectrum_to_fftw_indices.assign(known_spectrum_size, {-1, -1});

	for (int i = 0; i < fftw.output_size(); ++i)
	{
		const int spectrum_index =
			std::clamp((int)(calc_index_ratio(i) * known_spectrum_size), 0, known_spectrum_size - 1);
		if (spectrum_to_fftw_indices[spectrum_index].first == -1)
			spectrum_to_fftw_indices[spectrum_index].first = i;
		spectrum_to_fftw_indices[spectrum_index].second = i + 1;
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

void FrequencyAnalyzer::interpolate(std::span<float> spectrum)
{
	if (interp == InterpolationType::NONE || scale == Scale::LINEAR)
		return;

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

	using spline_type = tk::spline::spline_type;
	static const spline_type spline_type_table[] = {
		(spline_type)0, spline_type::linear, spline_type::cspline, spline_type::cspline_hermite};

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
