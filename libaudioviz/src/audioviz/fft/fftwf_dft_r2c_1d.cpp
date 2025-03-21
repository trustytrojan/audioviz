#include <audioviz/fft/fftwf_dft_r2c_1d.hpp>
#include <stdexcept>

namespace audioviz::fft
{

void fftwf_dft_r2c_1d::init(const int N)
{
	this->N = N;
	in = (float *)fftwf_malloc(sizeof(float) * N);
	out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * output_size());
	p = fftwf_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
}

void fftwf_dft_r2c_1d::cleanup()
{
	fftwf_destroy_plan(p);
	fftwf_free(in);
	fftwf_free(out);
}

void fftwf_dft_r2c_1d::set_n(const int N)
{
	if (!N)
		throw std::invalid_argument("N is zero");
	if (this->N == N)
		return;
	cleanup();
	init(N);
}

} // namespace audioviz::fft
