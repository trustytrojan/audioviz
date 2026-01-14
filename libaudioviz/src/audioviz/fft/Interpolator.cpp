#include <audioviz/fft/Interpolator.hpp>

namespace audioviz
{

void Interpolator::interpolate(std::span<float> range)
{
	const auto size = range.size();

	// separate the nonzero values (y's) and their indices (x's)
	m_spline_x.clear();
	m_spline_y.clear();
	zero_indices.clear();
	for (size_t i = 0; i < size; ++i)
	{
		if (!range[i])
		{
			zero_indices.emplace_back(i);
			continue;
		}
		m_spline_x.emplace_back(i);
		m_spline_y.emplace_back(range[i]);
	}

	// tk::spline::set_points throws if there are less than 3 points
	if (m_spline_x.size() < 3)
		return;

	spline.set_points(m_spline_x, m_spline_y, (tk::spline::spline_type)type);

	// only copy values to fill in the gaps (the zero valued indices)
	for (const auto zero_index : zero_indices)
		range[zero_index] = spline(zero_index);
}

} // namespace audioviz
