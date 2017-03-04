#pragma once

#include "scene_renderstate.hpp"
#include "base/math/vector.inl"

namespace scene {

inline float3 Renderstate::tangent_to_world(float3_p v) const{
	return float3(v.v[0] * t.v[0] + v.v[1] * b.v[0] + v.v[2] * n.v[0],
				  v.v[0] * t.v[1] + v.v[1] * b.v[1] + v.v[2] * n.v[1],
				  v.v[0] * t.v[2] + v.v[1] * b.v[2] + v.v[2] * n.v[2]);
}

inline float3 Renderstate::tangent_to_world(float2 v) const{
	return float3(v.v[0] * t.v[0] + v.v[1] * b.v[0],
				  v.v[0] * t.v[1] + v.v[1] * b.v[1],
				  v.v[0] * t.v[2] + v.v[1] * b.v[2]);
}

}
