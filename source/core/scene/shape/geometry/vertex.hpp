#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Vertex {
	math::float3 p, n, t;
//	float bitangent_sign;
	math::float2 uv;


//	math::float3 p; float pad0;
//	math::float3 n; float pad1;
//	math::float3 t; float pad2;

//	math::float2 uv; float pad3[2];
};

struct Data_vertex {
	math::float3 n, t;
	math::float2 uv;
};

}}
