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

	std::vector<Distribution_1D> conditional_;

	Distribution_1D marginal_;
};

}
