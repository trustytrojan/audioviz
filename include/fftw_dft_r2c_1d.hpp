#pragma once

#include <stdexcept>
#include <fftw3.h>

namespace fftw
{
	template <typename _Tp>
	class dft_r2c_1d;

	class dft_r2c_1d<float>
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
		dft_r2c_1d(const int N) { init(N); }
		~dft_r2c_1d() { cleanup(); }

		void set_n(const int N)
		{
			if (!N)
				throw std::invalid_argument("N is zero");
			if (this->N == N)
				return;
			cleanup();
			init(N);
		}

		void execute() { fftwf_execute(p); }
		float *input() { return in; }
		const fftwf_complex *output() const { return out; }
		int input_size() const { return N; }
		int output_size() const { return N / 2 + 1; }
	};

	class dft_r2c_1d<double>
	{
		int N;
		double *in;
		fftw_complex *out;
		fftw_plan p;

		void init(const int N)
		{
			this->N = N;
			in = (double *)fftw_malloc(sizeof(double) * N);
			out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * output_size());
			p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
		}

		void cleanup()
		{
			fftw_destroy_plan(p);
			fftw_free(in);
			fftw_free(out);
		}

	public:
		dft_r2c_1d(const int N) { init(N); }
		~dft_r2c_1d() { cleanup(); }

		void set_n(const int N)
		{
			if (!N)
				throw std::invalid_argument("N is zero");
			if (this->N == N)
				return;
			cleanup();
			init(N);
		}

		void execute() { fftw_execute(p); }
		double *input() { return in; }
		const fftw_complex *output() const { return out; }
		int input_size() const { return N; }
		int output_size() const { return N / 2 + 1; }
	};
}
