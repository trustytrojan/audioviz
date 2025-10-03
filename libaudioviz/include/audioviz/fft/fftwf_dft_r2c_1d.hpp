#pragma once

#include <fftw3.h>

namespace audioviz::fft
{

class fftwf_dft_r2c_1d
{
	int N, outN;
	float *in;
	fftwf_complex *out;
	fftwf_plan p;

	void init(const int N);
	void cleanup();

public:
	inline fftwf_dft_r2c_1d(const int N) { init(N); }
	inline ~fftwf_dft_r2c_1d() { cleanup(); }

	void set_n(const int N);
	inline void execute() { fftwf_execute(p); }
	inline float *input() { return in; }
	inline const fftwf_complex *output() const { return out; }
	inline int input_size() const { return N; }
	inline int output_size() const { return outN; }
};

} // namespace audioviz::fft
