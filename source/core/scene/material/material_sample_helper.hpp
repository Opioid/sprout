#ifndef SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLE_HELPER_HPP

#include "base/math/vector3.inl"

namespace scene::material {

constexpr float Dot_min = 0.00001f;

static inline float clamp(float x) {
	return std::clamp(x, Dot_min, 1.f);
}

static inline float clamp_dot(f_float3 a, f_float3 b) {
	return std::clamp(math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_reverse_dot(f_float3 a, f_float3 b) {
	return std::clamp(-math::dot(a, b), Dot_min, 1.f);
}

static inline float clamp_abs_dot(f_float3 a, f_float3 b) {
	return std::clamp(std::abs(math::dot(a, b)), Dot_min, 1.f);
}

}

#endif
