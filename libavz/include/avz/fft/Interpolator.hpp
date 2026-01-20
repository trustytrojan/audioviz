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
public:
	enum class InterpolationType
	{
		LINEAR = tk::spline::linear,
		CSPLINE = tk::spline::cspline,
		CSPLINE_HERMITE = tk::spline::cspline_hermite
	};

private:
	tk::spline spline;
	std::vector<double> m_spline_x, m_spline_y;
	std::vector<size_t> zero_indices;
	InterpolationType type{InterpolationType::CSPLINE};

public:
	/**
	 * Set the interpolation type to use.
	 * @param type interpolation type
	 */
	void set_interp_type(InterpolationType type) { this->type = type; }

	/**
	 * Get the current interpolation type.
	 */
	InterpolationType get_interp_type() const { return type; }

	/**
	 * Interpolate spectrum data to fill gaps between non-zero values.
	 * Uses the interpolation type set via set_interp_type().
	 * @param spectrum spectrum data to interpolate in-place
	 */
	void interpolate(std::span<float> spectrum);

	/**
	 * Set the values for the spline interpolation.
	 * The X values are assumed to be 0, 1, 2, ...
	 * @param values Y values
	 */
	void set_values(std::span<const float> values);

	/**
	 * Sample the spline at a given X coordinate.
	 * @param x X coordinate (index)
	 * @return interpolated value
	 */
	inline float sample(float x) const { return spline(x); }
};

} // namespace audioviz
