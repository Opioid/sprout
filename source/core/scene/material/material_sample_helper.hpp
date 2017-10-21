#pragma once

#include "base/math/vector3.inl"

namespace scene::material {

constexpr float Dot_min = 0.00001f;

static inline float3 scattering_coefficient(const float3& color, float distance) {
	const float r = color[0] * distance;
	const float g = color[1] * distance;
	const float b = color[2] * distance;
	return float3(r > 0.f ? 1.f / r : 0.f,
				  g > 0.f ? 1.f / g : 0.f,
				  b > 0.f ? 1.f / b : 0.f);
}

static inline float3 absorption_coefficient(const float3& color, float distance) {
//	return scattering_coefficient(float3(1.f) - color, distance);
//	const float i = 1.f / distance;
//	const float r = color[0] * distance;
//	const float g = color[1] * distance;
//	const float b = color[2] * distance;
//	return float3(r > 0.f ? (1.f / r) - i : i,
//				  g > 0.f ? (1.f / g) - i : i,
//				  b > 0.f ? (1.f / b) - i : i);

	return (float3(1.f) - color) / distance;
}

static inline float clamp(float x) {
	return math::clamp(x, Dot_min, 1.f);
}

static inline float clamp_dot(const float3& a, const float3& b) {
	return math::clamp(math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_reverse_dot(const float3& a, const float3& b) {
	return math::clamp(-math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_abs_dot(const float3& a, const float3& b) {
	return math::clamp(std::abs(math::dot(a, b)), Dot_min, 1.f);
}

}
