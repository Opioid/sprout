#pragma once

#include "base/math/vector.hpp"
#include <vector>

namespace procedural { namespace starburst {

class Aperture {

public:

	Aperture(uint32_t num_blades);

	float evaluate(float2 xy, float resolution);

private:

	std::vector<float3> blades_;
};

}}
