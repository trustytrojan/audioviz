#pragma once

#include <cmath>
#include <functional>
#include <tk-spline.hpp>
#include <vector>
#include <span>

#include "fftw_allocator.hpp"
#include "fftwf_dft_r2c_1d.hpp"

namespace audioviz
{

/**
 * Analyzes wave data to produce a frequency spectrum. Allows further processing
 * of the resulting spectrum, such as scaling and interpolation.
 */
class FrequencyAnalyzer
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
		LINEAR,
		CSPLINE,
		CSPLINE_HERMITE
	};

	enum class AccumulationMethod
	{
		SUM,
		MAX
	};

	using WindowFunction = std::function<float(int, int)>;

	static const inline WindowFunction WF_HANNING = [](int i, int fft_size)
	{ return 0.5f * (1 - cos(2 * M_PI * i / (fft_size - 1))); },
									   WF_HAMMING = [](int i, int fft_size)
	{ return 0.54f - 0.46f * cos(2 * M_PI * i / (fft_size - 1)); },
									   WF_BLACKMAN = [](int i, int fft_size)
	{
		return 0.42f - 0.5f * cos(2 * M_PI * i / (fft_size - 1)) + 0.08f * cos(4 * M_PI * i / (fft_size - 1));
	};

private:
	// fft size
	int fft_size;
	float inv_fft_size;

	// nth root
	int nth_root{2};
	float nthroot_inv{1.f / nth_root};

	fftwf_dft_r2c_1d fftw{fft_size};

	// interpolation
	tk::spline spline;
	InterpolationType interp{InterpolationType::CSPLINE};

	// output spectrum scale
	Scale scale{Scale::LOG};

	// method for accumulating amplitudes in frequency bins
	AccumulationMethod am{AccumulationMethod::MAX};

	// window function
	WindowFunction window_func{WF_BLACKMAN};
	int wf_i{3}; // for imgui

	// struct to hold the "max"s used in `calc_index_ratio`
	struct _scale_max
	{
		float linear, log, sqrt, cbrt, nthroot;
		void calc(int new_max, float nthroot_inv);
	} scale_max;

	int bin_pack_input_size{};
	std::vector<std::pair<int, int>> bin_pack_index_mapping;
	std::vector<float, fftw_allocator<float>> window_values;
	std::vector<double> m_spline_x, m_spline_y;

public:
	/**
	 * Initialize frequency spectrum renderer.
	 * @param fft_size sample chunk size used by FFT processor
	 */
	FrequencyAnalyzer(int fft_size);

	/**
	 * Set the FFT size used in the backend library.
	 * @param fft_size new fft size to use
	 * @returns reference to self
	 * @throws `std::invalid_argument` if `fft_size` is not even
	 */
	void set_fft_size(int fft_size);
	inline int get_fft_size() const { return fft_size; }
	inline int get_fft_output_size() const { return fftw.output_size(); }

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

#ifdef AUDIOVIZ_IMGUI
	/**
	 * Draw ImGui controls for tuning analyzer parameters at runtime.
	 * This calls the public setters so changes take effect immediately.
	 */
	void draw_imgui();
#endif

	/**
	 * Copies the `wavedata` to the FFT processor for rendering.
	 * @param wavedata input wave sample data, expected to be of size `fft_size`
	 */
	void copy_to_input(std::span<const float> wavedata);

	/**
	 * Copies a specific channel of the audio to the FFT processor, which is of size `fft_size`.
	 * If `num_channels` is greater than 1, then `audio` is expected to be of size `num_channels * fft_size`.
	 * @throws `std::invalid_argument` if `channel` is not in the range `[0, num_channels)`
	 * @throws `std::invalid_argument` if `num_channels <= 0`
	 */
	void copy_channel_to_input(std::span<const float> audio, int num_channels, int channel, bool interleaved);

	inline void execute_fft() const { fftw.execute(); }
	void compute_amplitude(std::span<float> output);
	void bin_pack(std::span<float> out, std::span<const float> in);
	void interpolate(std::span<float> spectrum);

private:
	float calc_index_ratio(float i) const;
	void compute_bin_pack_index_mappings(size_t out_size, size_t in_size);
	void compute_window_values();
};

} // namespace audioviz
