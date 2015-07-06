#pragma once

#include "distribution_1d.hpp"
#include "vector.hpp"

namespace math {

class Distribution_2D {
public:

	void init(const float* data, const math::uint2& dimensions);

	math::float2 sample_continuous(math::float2 uv, float& pdf) const;

private:

	std::vector<Distribution_1D> conditional_;

	Distribution_1D marginal_;

	float area_;
};

}
