#pragma once

#include "base/math/vector3.inl"

namespace scene { namespace material {

constexpr float Dot_min = 0.00001f;

static inline float3 attenuation(const float3& color, float distance) {
	const float3 pushed = color + float3(0.01f);
	return float3(1.f / (pushed[0] * distance),
				  1.f / (pushed[1] * distance),
				  1.f / (pushed[2] * distance));
}

static inline float clamped_dot(const float3& a, const float3& b) {
	return math::clamp(math::dot(a, b), Dot_min, 1.f);
}

static inline float reversed_clamped_dot(const float3& a, const float3& b) {
	return math::clamp(-math::dot(a, b), Dot_min, 1.f);
}

static inline float absolute_clamped_dot(const float3& a, const float3& b) {
	return math::clamp(std::abs(math::dot(a, b)), Dot_min, 1.f);
}

}}
