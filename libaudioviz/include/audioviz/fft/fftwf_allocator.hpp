#pragma once

#include <cstddef>
#include <fftw3.h>
#include <print>

template <typename T>
struct fftwf_allocator
{
	using value_type = T;

	inline T *allocate(const size_t n)
	{
		std::println("[fftwf_allocator::allocate] called");
		return (T *)fftwf_malloc(n * sizeof(T));
	}

	inline void deallocate(T *const p, size_t)
	{
		std::println("[fftwf_allocator::deallocate] called");
		fftwf_free(p);
	}
};
