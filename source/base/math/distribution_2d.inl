#pragma once

#include "distribution_2d.hpp"

namespace math {

void Distribution_2D::init(const float* data, const math::uint2& dimensions) {

}

math::float2 Distribution_2D::sample_continuous(float u, float v, float& pdf) const {
	return math::float2(0.5f, 0.5f);
}

}

