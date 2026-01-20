#pragma once

#include <span>
#include <vector>

namespace avz
{

/**
 * Packs frequency spectrum bins using various scaling methods and accumulation strategies.
 * Maintains state for efficient index mapping computations.
 */
class BinPacker
{
public:
	enum class Scale
	{
		LINEAR,
		LOG,
		NTH_ROOT
	};

	enum class AccumulationMethod
	{
		SUM,
		MAX
	};

private:
	// output spectrum scale
	Scale scale{Scale::LOG};

	// nth root for NTH_ROOT scale
	int nth_root{2};
	float nthroot_inv{1.f / nth_root};

	// method for accumulating amplitudes in frequency bins
	AccumulationMethod am{AccumulationMethod::MAX};

	// struct to hold the "max"s used in `calc_index_ratio`
	struct _scale_max
	{
		float linear, log, sqrt, cbrt, nthroot;
		void calc(int new_max, float nthroot_inv);
	} scale_max;

	// state for bin packing
	int bin_pack_input_size{};
	std::vector<std::pair<int, int>> bin_pack_index_mapping;

public:
	BinPacker() = default;

	/**
	 * Set the spectrum's frequency scale.
	 * @param scale new scale to use
	 */
	void set_scale(Scale scale);

	/**
	 * Set the nth-root to use when using the `NTH_ROOT` scale.
	 * @param nth_root new nth_root to use
	 * @throws `std::invalid_argument` if `nth_root` is zero
	 */
	void set_nth_root(int nth_root);

	/**
	 * Set frequency bin accumulation method.
	 * @param am new accumulation method to use
	 */
	void set_accum_method(AccumulationMethod am);

	/**
	 * Get the current scale.
	 */
	inline Scale get_scale() const { return scale; }

	/**
	 * Get the current nth root value.
	 */
	inline int get_nth_root() const { return nth_root; }

	/**
	 * Get the current accumulation method.
	 */
	inline AccumulationMethod get_accum_method() const { return am; }

	/**
	 * Pack frequency bins from input to output spectrum using configured scale and accumulation method.
	 * @param out output spectrum (smaller size)
	 * @param in input spectrum (larger size)
	 */
	void bin_pack(std::span<float> out, std::span<const float> in);

private:
	float calc_index_ratio(float i) const;
	void compute_bin_pack_index_mappings(size_t out_size, size_t in_size);
};

} // namespace avz
