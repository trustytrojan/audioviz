#pragma once

#include "fftwf_allocator.hpp"
#include <fftw3.h>
#include <span>
#include <vector>

namespace audioviz
{

class fftwf_dft_r2c_1d
{
public:
	struct ComplexNumber
	{
		float re, im;
	};

private:
	template <typename T>
	using Vector = std::vector<T, fftwf_allocator<T>>;

	Vector<float> in;
	Vector<ComplexNumber> out;
	fftwf_plan plan{};

public:
	inline ~fftwf_dft_r2c_1d()
	{
		if (plan)
			fftwf_destroy_plan(plan);
	}

	inline void set_n(const int n)
	{
		in.resize(n);
		out.resize(n / 2 + 1);
		plan = fftwf_plan_dft_r2c_1d(n, in.data(), (fftwf_complex *)out.data(), FFTW_ESTIMATE);
	}

	inline void execute() const { fftwf_execute(plan); }
	inline std::span<float> input() { return in; }
	inline std::span<const ComplexNumber> output() const { return out; }
	inline int output_size() const { return out.size(); }
};

} // namespace audioviz
