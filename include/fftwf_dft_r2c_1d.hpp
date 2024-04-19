#pragma once

#include <stdexcept>
#include <fftw3.h>

class fftwf_dft_r2c_1d
{
	int N;
	float *in;
	fftwf_complex *out;
	fftwf_plan p;

	void init(const int N)
	{
		this->N = N;
		in = (float *)fftwf_malloc(sizeof(float) * N);
		out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * output_size());
		p = fftwf_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
	}

	void cleanup()
	{
		fftwf_destroy_plan(p);
		fftwf_free(in);
		fftwf_free(out);
	}

public:
	fftwf_dft_r2c_1d(const int N) { init(N); }
	~fftwf_dft_r2c_1d() { cleanup(); }

	void set_n(const int N)
	{
		if (!N)
			throw std::invalid_argument("N is zero");
		if (this->N == N) return;
		cleanup();
		init(N);
	}

	void execute() { fftwf_execute(p); }
	float *input() { return in; }
	const fftwf_complex *output() const { return out; }
	int input_size() const { return N; }
	int output_size() const { return N / 2 + 1; }
};