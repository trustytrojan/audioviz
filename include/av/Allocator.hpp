#pragma once

#include <stdexcept>
#include <new>

extern "C"
{
#include <libavutil/avstring.h>
#include <libavutil/avutil.h>
}

namespace av
{
	template <typename _Tp>
	class Allocator
	{
	public:
		using value_type = _Tp;

		Allocator() = default;

		template <typename U>
		Allocator(const Allocator<U> &) noexcept {}

		_Tp *allocate(std::size_t n)
		{
			if (n > std::size_t(-1) / sizeof(_Tp))
				throw std::bad_alloc();

			if (auto p = static_cast<_Tp *>(av_malloc(n * sizeof(_Tp))))
				return p;

			throw std::bad_alloc();
		}

		void deallocate(_Tp *p, std::size_t) noexcept
		{
			av_free(p);
		}
	};
}
