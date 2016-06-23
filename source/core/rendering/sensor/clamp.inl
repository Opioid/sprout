#pragma once

#include "clamp.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace clamp {

inline float4 Identity::clamp(const float4& color) const {
	return color;
}

inline Clamp::Clamp(const float3& max) : max_(max) {}

inline float4 Clamp::clamp(const float4& color) const {
	return float4(std::min(color.x, max_.x), std::min(color.y, max_.y), std::min(color.z, max_.z), color.w);
}

}}}
