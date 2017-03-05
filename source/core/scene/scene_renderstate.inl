#pragma once

#include "scene_renderstate.hpp"
#include "base/math/vector.inl"

namespace scene {

inline float3 Renderstate::tangent_to_world(float3_p v) const{
	return float3(v[0] * t[0] + v[1] * b[0] + v[2] * n[0],
				  v[0] * t[1] + v[1] * b[1] + v[2] * n[1],
				  v[0] * t[2] + v[1] * b[2] + v[2] * n[2]);
}

inline float3 Renderstate::tangent_to_world(float2 v) const{
	return float3(v[0] * t[0] + v[1] * b[0],
				  v[0] * t[1] + v[1] * b[1],
				  v[0] * t[2] + v[1] * b[2]);
}

}
