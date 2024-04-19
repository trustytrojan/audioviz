#pragma once

#include <cstring>
#include <vector>
#include "spline.hpp"
#include "fftwf_dft_r2c_1d.hpp"

class FrequencySpectrum
{
public:
	enum class Scale
	{
		LINEAR,
		LOG,
		NTH_ROOT
	};

	enum class InterpolationType
	{
		NONE,
		LINEAR = tk::spline::linear,
		CSPLINE = tk::spline::cspline,
		CSPLINE_HERMITE = tk::spline::cspline_hermite
	};

	enum class AccumulationMethod
	{
		SUM,
		MAX
	};

	enum class WindowFunction
	{
		NONE,
		HANNING,
		HAMMING,
		BLACKMAN
	};

private:
	// fft size
	int fft_size;

	// nth root
	int nth_root = 2;
	float nthroot_inv = 1.f / nth_root;

	// fftw initialization
	fftwf_dft_r2c_1d fftw = fft_size;

	// interpolation
	tk::spline spline;
	InterpolationType interp = InterpolationType::CSPLINE;

	// output spectrum scale
	Scale scale = Scale::LOG;

	// method for accumulating amplitudes in frequency bins
	AccumulationMethod am = AccumulationMethod::MAX;

	// window function
	WindowFunction wf = WindowFunction::BLACKMAN;

	// struct to hold the "max"s used in `calc_index_ratio`
	struct
	{
		double linear, log, sqrt, cbrt, nthroot;
		void set(const FrequencySpectrum &fs)
		{
			const auto max = fs.fftw.output_size();
			linear = max;
			log = ::log(max);
			sqrt = ::sqrt(max);
			cbrt = ::cbrt(max);
			nthroot = ::pow(max, fs.nthroot_inv);
		}
	} scale_max;

public:
	/**
	 * Initialize frequency spectrum renderer.
	 * @param fft_size sample chunk size fed into the `transform` method
	 */
	FrequencySpectrum(int fft_size);

	/**
	 * Set the FFT size used in the `kissfft` library.
	 * @param fft_size new fft size to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `fft_size` is not even
	 */
	void set_fft_size(int fft_size);

	/**
	 * Set interpolation type.
	 * @param interp new interpolation type to use
	 * @returns reference to self
	 */
	void set_interp_type(InterpolationType interp);

	/**
	 * Set window function.
	 * @param interp new window function to use
	 * @returns reference to self
	 */
	void set_window_func(WindowFunction wf);

	/**
	 * Set frequency bin accumulation method.
	 * @param interp new accumulation method to use
	 * @returns reference to self
	 */
	void set_accum_method(AccumulationMethod am);

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 * @returns reference to self
	 */
	void set_scale(Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	void set_nth_root(int nth_root);

	/**
	 * Copies the `wavedata` to the FFTW input buffer for rendering.
	 * @param wavedata input wave sample data, expected to be of size `fft_size`
	 */
	void copy_to_input(const float *wavedata)
	{
		memcpy(fftw.input(), wavedata, fft_size * sizeof(float));
	}

	/**
	 * This method is meant for audio data.
	 * Copies a specific channel of the audio buffer to the FFTW input buffer, which is of size `fft_size`.
	 * If `num_channels` is greater than 1, then `audio` is expected to be of size `num_channels * fft_size`.
	 * @throws `std::invalid_argument` if `channel` is not in the range `[0, num_channels)`
	 * @throws `std::invalid_argument` if `num_channels <= 0`
	 */
	void copy_channel_to_input(const float *audio, int num_channels, int channel, bool interleaved);

	/**
	 * Performs the FFT on the wave data copied via `copy_channel_to_input`.
	 * @param spectrum 
	 */
	void render(std::vector<float> &spectrum);

private:
	float window_func(int i);
	int calc_index(int i, int max_index);
	float calc_index_ratio(float i);
	void interpolate(std::vector<float> &spectrum);
};