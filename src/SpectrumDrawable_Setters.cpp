#include "SpectrumDrawable.hpp"

void SpectrumDrawable::set_multiplier(float multiplier)
{
	this->multiplier = multiplier;
}

void SpectrumDrawable::set_bar_width(const int width)
{
	bar.width = width;
}

void SpectrumDrawable::set_bar_spacing(const int spacing)
{
	bar.spacing = spacing;
}

void SpectrumDrawable::set_fft_size(int fft_size)
{
	fs.set_fft_size(fft_size);
}

void SpectrumDrawable::set_interp_type(FS::InterpolationType interp_type)
{
	fs.set_interp_type(interp_type);
}

void SpectrumDrawable::set_scale(FS::Scale scale)
{
	fs.set_scale(scale);
}

void SpectrumDrawable::set_nth_root(int nth_root)
{
	fs.set_nth_root(nth_root);
}

void SpectrumDrawable::set_accum_method(FS::AccumulationMethod method)
{
	fs.set_accum_method(method);
}

void SpectrumDrawable::set_window_func(FS::WindowFunction wf)
{
	fs.set_window_func(wf);
}