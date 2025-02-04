#include "viz/util.hpp"

namespace viz::util
{

float weighted_max(
	const std::vector<float> &vec,
	const std::function<float(float)> &weight_func,
	const float size_divisor) // generally the lower third of the frequency spectrum is considered bass
{
	/**
	 * TODO: rewrite weighted max function to have slightly lower weight as frequencies approach 20hz,
	 * aka the first spectrum bar/element in either data vector.
	 */

	const auto _weighted_max = [&](const auto begin, const auto end, const auto weight_start)
	{
		auto max_value = *begin;
		const auto total_distance = static_cast<float>(std::distance(weight_start, end));
		for (auto it = begin; it < end; ++it)
		{
			// weight each element before it is compared
			const auto unweighted = std::distance(it, end) / total_distance;

			// clang-format off
			const auto weight = (it < weight_start)
				? 1.f
				: (weight_func ? weight_func(unweighted) : unweighted);
			// clang-format on

			const auto value = *it * weight;

			if (value > max_value)
				max_value = value;
		}
		return max_value;
	};

	const auto begin = vec.begin();
	const auto amount = vec.size() / size_divisor;
	return _weighted_max(
		begin,
		begin + amount,		   // only the first 50% of the range will have full weight
		begin + (amount / 2)); // these are generally the strongest bass frequencies to the ear
}

} // namespace viz::util
