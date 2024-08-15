#pragma once

#include <cmath>
#include <functional>
#include <vector>

namespace viz::util
{

float weighted_max(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func = {},
	const float size_divisor = 3.5f); // generally the lower third of the frequency spectrum is considered bass

} // namespace viz::util
