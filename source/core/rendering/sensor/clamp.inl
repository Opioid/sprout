#pragma once

#include "clamp.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace clamp {

inline math::float4 Identity::clamp(const math::float4& color) const {
	return color;
}

inline Clamp::Clamp(const math::vec3& max) : max_(max) {}

inline math::float4 Clamp::clamp(const math::float4& color) const {
	return math::float4(std::min(color.x, max_.x), std::min(color.y, max_.y), std::min(color.z, max_.z), color.w);
}

}}}
