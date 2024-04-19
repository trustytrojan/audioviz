#include "FrequencySpectrum.hpp"
#include <stdexcept>

void FrequencySpectrum::set_fft_size(const int fft_size)
{
	this->fft_size = fft_size;
	fftw.set_n(fft_size);
	scale_max.set(*this);
}

void FrequencySpectrum::set_interp_type(const InterpolationType interp)
{
	this->interp = interp;
}

void FrequencySpectrum::set_window_func(const WindowFunction wf)
{
	this->wf = wf;
}

void FrequencySpectrum::set_accum_method(const AccumulationMethod am)
{
	this->am = am;
}

void FrequencySpectrum::set_scale(const Scale scale)
{
	this->scale = scale;
}

void FrequencySpectrum::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument("FrequencySpectrun::set_nth_root: nth_root cannot be zero!");
	this->nth_root = nth_root;
	nthroot_inv = 1.f / nth_root;
}