#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Vertex {
	math::packed_float3 p;
	math::packed_float3 n;
	math::packed_float3 t;
	float2 uv;
	float bitangent_sign;
};

}}
