#pragma once

#include "base/math/vector.hpp"
#include <vector>

namespace procedural { namespace starburst {

class Aperture {

public:

	Aperture(uint32_t num_blades, float roundness);

	float evaluate(float2 p) const;

private:

	std::vector<float3> blades_;

	float roundness_;
};

}}
