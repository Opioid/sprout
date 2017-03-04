#pragma once

#include "clamp.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace clamp {

inline float4 Identity::clamp(float4_p color) const {
	return color;
}

inline Clamp::Clamp(float3_p max) : max_(max) {}

inline float4 Clamp::clamp(float4_p color) const {
	return float4(std::min(color.v[0], max_.v[0]),
				  std::min(color.v[1], max_.v[1]),
				  std::min(color.v[2], max_.v[2]),
				  color.v[3]);
}

}}}
