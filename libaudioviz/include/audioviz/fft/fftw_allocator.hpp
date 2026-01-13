#pragma once

#include <cstddef>
#include <fftw3.h>

template <typename T>
struct fftw_allocator
{
	using value_type = T;
	inline T *allocate(const size_t n) { return (T *)fftwf_malloc(n * sizeof(T)); }
	inline void deallocate(T *const p, size_t) { fftwf_free(p); }
};
