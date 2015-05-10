#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Vertex {
	math::float3 p, n, t;
//	float bitangent_sign;
	math::float2 uv;
};

}}
