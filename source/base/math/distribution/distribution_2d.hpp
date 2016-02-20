#pragma once

#include "distribution_1d.hpp"
#include "math/vector.hpp"

namespace math {

class Distribution_2D {
public:

	void init(const float* data, const math::int2& dimensions);

	math::float2 sample_continuous(math::float2 r2, float& pdf) const;

	float pdf(math::float2 uv) const;

private:

//	typedef Distribution_1D Distribution_impl;
//	typedef Distribution_lut_1D<8> Distribution_impl;
	typedef Distribution_luty_1D Distribution_impl;

	Distribution_impl marginal_;

	std::vector<Distribution_impl> conditional_;

	float conditional_max_;
};

}
