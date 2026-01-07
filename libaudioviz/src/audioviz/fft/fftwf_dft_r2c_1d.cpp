#include <audioviz/fft/fftwf_dft_r2c_1d.hpp>
#include <stdexcept>

// forget about the more accurate fftw plans & wisdom for now,
// we can add ui elements for that later on, and FFTW_ESTIMATE gets us great results anyway

// const auto WISDOM_FILENAME = "audioviz.wisdom";

namespace audioviz
{

void fftwf_dft_r2c_1d::init(const int N)
{
	this->N = N;
	this->outN = N / 2 + 1;
	in = (float *)fftwf_malloc(sizeof(float) * N);
	out = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * output_size());
	// fftwf_import_wisdom_from_filename(WISDOM_FILENAME);
	p = fftwf_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
	// fftwf_export_wisdom_to_filename(WISDOM_FILENAME);
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

} // namespace audioviz
