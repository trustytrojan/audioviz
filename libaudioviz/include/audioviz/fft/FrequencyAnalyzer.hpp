#pragma once

#include <cmath>
#include <functional>
#include <span>
#include <vector>

#include "fftwf_allocator.hpp"
#include "fftwf_dft_r2c_1d.hpp"

namespace audioviz
{

/**
 * Analyzes wave data to produce a frequency spectrum using FFT.
 * Focuses solely on FFT operations and amplitude computation.
 * Use BinPacker and Interpolator classes for further spectrum processing.
 */
class FrequencyAnalyzer
{
public:
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

	fftwf_dft_r2c_1d fftw;

	// window function
	WindowFunction window_func{WF_BLACKMAN};
	int wf_i{3}; // for imgui

	std::vector<float, fftwf_allocator<float>> window_values;

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
	 * Set window function.
	 * @param wf new window function to use
	 */
	void set_window_func(WindowFunction wf);

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

	inline void execute_fft() const { fftw.execute(); }
	void compute_amplitude(std::span<float> output) const;
	void compute_phase(std::span<float> output) const;

private:
	void compute_window_values(bool force = {});
};

} // namespace audioviz
