#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>

// _aligned_malloc/_aligned_free on MSVC
#if defined(_MSC_VER) || defined(_WIN32)
#include <malloc.h>
#endif

template <typename T, size_t Alignment = 32>
struct aligned_allocator
{
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;
	using propagate_on_container_copy_assignment = std::true_type;
	using propagate_on_container_move_assignment = std::true_type;
	using is_always_equal = std::true_type;

	template <typename U>
	struct rebind
	{
		using other = aligned_allocator<U, Alignment>;
	};

	static_assert(Alignment > 0 && (Alignment & (Alignment - 1)) == 0, "Alignment must be a power of 2");

	T *allocate(size_t n)
	{
		if (n == 0)
			return nullptr;
		void *ptr = nullptr;
		size_t size = n * sizeof(T);
		// std::aligned_alloc requires size to be a multiple of alignment
		if (size % Alignment != 0)
			size += Alignment - (size % Alignment);

#if defined(_MSC_VER) || defined(_WIN32)
		ptr = _aligned_malloc(size, Alignment);
		if (!ptr)
			throw std::bad_alloc();
#elif defined(__APPLE__)
		// macOS doesn't provide aligned_alloc in libc; use posix_memalign
		if (posix_memalign(&ptr, Alignment, size) != 0)
			throw std::bad_alloc();
#else
		ptr = std::aligned_alloc(Alignment, size);
		if (!ptr)
			throw std::bad_alloc();
#endif

		return static_cast<T *>(ptr);
	}

	void deallocate(T *p, size_t)
	{
#if defined(_MSC_VER) || defined(_WIN32)
		_aligned_free(p);
#else
		free(p);
#endif
	}

	template <typename U, size_t OtherAlignment>
	bool operator==(const aligned_allocator<U, OtherAlignment> &) const noexcept
	{
		return Alignment == OtherAlignment;
	}

	template <typename U, size_t OtherAlignment>
	bool operator!=(const aligned_allocator<U, OtherAlignment> &) const noexcept
	{
		return Alignment != OtherAlignment;
	}
};
