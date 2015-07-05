#pragma once

#include "distribution_1d.hpp"
#include "vector.hpp"

namespace math {

class Distribution_2D {
public:

	void init(const float* data, const math::uint2& dimensions);

	math::float2 sample_continuous(float u, float v, float& pdf) const;

private:

	std::vector<Distribution_1D> conditional_;
};

}
