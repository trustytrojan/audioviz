#pragma once

#include <vector>
#include "Allocator.hpp"

namespace av
{
	template <typename _Tp>
	using Vector = std::vector<_Tp, Allocator<_Tp>>;
}