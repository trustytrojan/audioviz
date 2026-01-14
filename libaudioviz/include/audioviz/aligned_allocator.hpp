#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>

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
		if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0)
			throw std::bad_alloc();
		return static_cast<T *>(ptr);
	}

	void deallocate(T *p, size_t) { free(p); }

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
