#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_MATERIAL_INL

#include "base/math/vector2.inl"
#include "material.hpp"

namespace scene::material {

inline Material::Radiance_sample::Radiance_sample(float2 uv, float pdf)
    : uvw{uv[0], uv[1], 0.f, pdf} {}

inline Material::Radiance_sample::Radiance_sample(float3 const& uvw, float pdf)
    : uvw{uvw[0], uvw[1], uvw[2], pdf} {}

inline float Material::Radiance_sample::pdf() const {
    return uvw[3];
}

}  // namespace scene::material

#endif
