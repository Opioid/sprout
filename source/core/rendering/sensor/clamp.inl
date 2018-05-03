#pragma once

#include "clamp.hpp"
#include "base/math/vector3.inl"

namespace rendering::sensor::clamp {

inline float4 Identity::clamp(float4 const& color) const {
	return color;
}

inline Clamp::Clamp(float3 const& max) : max_(max) {}

inline float4 Clamp::clamp(float4 const& color) const {
	return float4(std::min(color[0], max_[0]),
				  std::min(color[1], max_[1]),
				  std::min(color[2], max_[2]),
				  color[3]);
}

}
