#pragma once

#include "base/math/vector3.hpp"

namespace scene { namespace shape {

struct Vertex {
	packed_float3 p;
	packed_float3 n;
	packed_float3 t;
	float2 uv;
	float bitangent_sign;
};

}}
