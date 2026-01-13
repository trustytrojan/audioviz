#pragma once

#include <span>
#include <tk-spline.hpp>
#include <vector>

namespace audioviz
{

/**
 * Handles interpolation of frequency spectrum data using various methods.
 */
class Interpolator
{
	tk::spline spline;
	std::vector<double> m_spline_x, m_spline_y;
	std::vector<size_t> zero_indices;

public:
	enum class InterpolationType
	{
		LINEAR = tk::spline::linear,
		CSPLINE = tk::spline::cspline,
		CSPLINE_HERMITE = tk::spline::cspline_hermite
	};

	/**
	 * Interpolate spectrum data to fill gaps between non-zero values.
	 * Only applies interpolation if type is not NONE and scale is not LINEAR.
	 * @param spectrum spectrum data to interpolate in-place
	 * @param scale_is_linear whether the scale is linear (disables interpolation)
	 */
	void interpolate(std::span<float> spectrum, InterpolationType type = InterpolationType::CSPLINE);
};

} // namespace audioviz
