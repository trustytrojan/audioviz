#pragma once

#include <cstddef>
#include <fftw3.h>

template <typename T>
class fftw_allocator
{
public:
	using value_type = T;

	template <typename U>
	struct rebind
	{
		using other = fftw_allocator<U>;
	};

	T *allocate(size_t n)
	{
		return static_cast<T *>(fftwf_malloc(n * sizeof(T)));
	}

	void deallocate(T *p, size_t)
	{
		fftwf_free(p);
	}
};
