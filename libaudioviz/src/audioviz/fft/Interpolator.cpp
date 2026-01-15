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

void Interpolator::set_values(std::span<const float> values)
{
	m_spline_x.clear();
	m_spline_y.clear();
	m_spline_x.reserve(values.size());
	m_spline_y.reserve(values.size());

	for (size_t i = 0; i < values.size(); ++i)
	{
		m_spline_x.emplace_back(i);
		m_spline_y.emplace_back(values[i]);
	}

	if (m_spline_x.size() >= 3)
		spline.set_points(m_spline_x, m_spline_y, (tk::spline::spline_type)type);
}

float Interpolator::sample(float x) const
{
	if (m_spline_x.size() < 3)
		return 0.0f;
	return std::max(0.0, spline(x));
}

} // namespace audioviz
