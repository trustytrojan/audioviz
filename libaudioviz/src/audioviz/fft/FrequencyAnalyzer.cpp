#include <audioviz/fft/FrequencyAnalyzer.hpp>
#include <cstring>
#include <stdexcept>

namespace audioviz::fft
{

void FrequencyAnalyzer::_scale_max::calc(const FrequencyAnalyzer &fa)
{
	const auto max = fa.fftw.output_size();
	linear = max;
	log = ::log(max);
	sqrt = ::sqrt(max);
	cbrt = ::cbrt(max);
	nthroot = ::pow(max, fa.nthroot_inv);
}

FrequencyAnalyzer::FrequencyAnalyzer(const int fft_size)
	: fft_size{fft_size}
{
	set_fft_size(fft_size);
	scale_max.calc(*this);
}

void FrequencyAnalyzer::set_fft_size(const int fft_size)
{
	this->fft_size = fft_size;
	fftw.set_n(fft_size);
	scale_max.calc(*this);
}

void FrequencyAnalyzer::set_interp_type(const InterpolationType interp)
{
	this->interp = interp;
}

void FrequencyAnalyzer::set_window_func(const WindowFunction wf)
{
	this->window_func = wf;
}

void FrequencyAnalyzer::set_accum_method(const AccumulationMethod am)
{
	this->am = am;
}

void FrequencyAnalyzer::set_scale(const Scale scale)
{
	this->scale = scale;
}

void FrequencyAnalyzer::set_nth_root(const int nth_root)
{
	if (!nth_root)
		throw std::invalid_argument("FrequencySpectrun::set_nth_root: nth_root cannot be zero!");
	this->nth_root = nth_root;
	nthroot_inv = 1.f / nth_root;
}

void FrequencyAnalyzer::copy_to_input(const float *const wavedata)
{
	memcpy(fftw.input(), wavedata, fft_size * sizeof(float));
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

	const auto input = fftw.input();
	for (int i = 0; i < fft_size; ++i)
		input[i] = audio[i * num_channels + channel];
}

void FrequencyAnalyzer::render(std::vector<float> &spectrum)
{
	assert(spectrum.size());
	assert(window_func);

	// apply window function on input
	const auto input = fftw.input();
	for (int i = 0; i < fft_size; ++i)
		input[i] *= window_func(i, fft_size);

	// execute fft and get output
	fftw.execute();
	const auto output = fftw.output();

	// zero out array since we are accumulating
	std::ranges::fill(spectrum, 0);

	// map frequency bins of freqdata to spectrum
	for (int i = 0; i < fftw.output_size(); ++i)
	{
		const auto [re, im] = output[i];
		// must divide by fft_size here to counteract the correlation
		// between fft_size and the average amplitude across the spectrum vector.
		const float amplitude = sqrt((re * re) + (im * im)) / fft_size;
		const auto index = calc_index(i, spectrum.size());

		switch (am)
		{
		case AccumulationMethod::SUM:
			spectrum[index] += amplitude;
			break;

		case AccumulationMethod::MAX:
			spectrum[index] = std::max(spectrum[index], amplitude);
			break;

		default:
			throw std::logic_error("FrequencySpectrum::render: switch(accum_type): default case hit");
		}
	}

	// apply interpolation if necessary
	if (interp != InterpolationType::NONE && scale != Scale::LINEAR)
		interpolate(spectrum);
}

int FrequencyAnalyzer::calc_index(const int i, const int max_index) const
{
	return std::max(0, std::min(int(calc_index_ratio(i) * max_index), max_index - 1));
}

float FrequencyAnalyzer::calc_index_ratio(const float i) const
{
	switch (scale)
	{
	case Scale::LINEAR:
		return i / scale_max.linear;
	case Scale::LOG:
		return log(i ? i : 1) / scale_max.log;
	case Scale::NTH_ROOT:
		switch (nth_root)
		{
		case 1:
			return i / scale_max.linear;
		case 2:
			return sqrt(i) / scale_max.sqrt;
		case 3:
			return cbrt(i) / scale_max.cbrt;
		default:
			return pow(i, nthroot_inv) / scale_max.nthroot;
		}
	default:
		throw std::logic_error("FrequencySpectrum::calc_index_ratio: default case hit");
	}
}

void FrequencyAnalyzer::interpolate(std::vector<float> &spectrum)
{
	// separate the nonzero values (y's) and their indices (x's)
	std::vector<double> nonzero_values, indices;
	for (int i = 0; i < (int)spectrum.size(); ++i)
	{
		if (!spectrum[i])
			continue;
		nonzero_values.push_back(spectrum[i]);
		indices.push_back(i);
	}

	// tk::spline::set_points throws if there are less than 3 points
	if (indices.size() < 3)
		return;

	spline.set_points(indices, nonzero_values, (tk::spline::spline_type)interp);

	// only copy spline values to fill in the gaps
	for (size_t i = 0; i < spectrum.size(); ++i)
		if (spectrum[i] == 0)
			spectrum[i] = spline(i);
}

} // namespace audioviz::fft
