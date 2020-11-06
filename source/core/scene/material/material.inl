#ifndef SU_CORE_SCENE_MATERIAL_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_MATERIAL_INL

#include "base/math/vector2.inl"
#include "collision_coefficients.inl"
#include "fresnel/fresnel.inl"
#include "image/texture/texture_adapter.inl"
#include "material.hpp"
#include "scene/scene_worker.hpp"

namespace scene::material {

inline float Material::opacity(float2 uv, uint64_t /*time*/, Filter filter,
                               Worker const& worker) const {
    if (mask_.is_valid()) {
        auto const& sampler = worker.sampler_2D(sampler_key_, filter);

        if (element_ < 0) {
            return mask_.sample_1(worker, sampler, uv);
        } else {
            return mask_.sample_1(worker, sampler, uv, element_);
        }
    }

    return 1.f;
}

inline float Material::border(float3 const& wi, float3 const& n) const {
    float const f0 = fresnel::schlick_f0(ior_, 1.f);

    float const n_dot_wi = std::max(dot(n, wi), 0.f);

    float const f = 1.f - fresnel::schlick(n_dot_wi, f0);

    return f;
}

inline CC Material::collision_coefficients() const {
    return cc_;
}

inline CC Material::collision_coefficients(float2 uv, Filter filter, Worker const& worker) const {
    if (color_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const color   = color_map_.sample_3(worker, sampler, uv);

        return scattering(cc_.a, color);
    }

    return cc_;
}

inline uint32_t Material::sampler_key() const {
    return sampler_key_;
}

inline bool Material::is_two_sided() const {
    return properties_.is(Property::Two_sided);
}

inline bool Material::is_masked() const {
    return mask_.is_valid();
}

inline bool Material::is_animated() const {
    return properties_.is(Property::Animated);
}

inline bool Material::is_caustic() const {
    return properties_.is(Property::Caustic);
}

inline bool Material::has_tinted_shadow() const {
    return properties_.is(Property::Tinted_shadow);
}

inline bool Material::has_emission_map() const {
    return properties_.is(Property::Emission_map);
}

inline bool Material::is_scattering_volume() const {
    return properties_.is(Property::Scattering_volume);
}

inline bool Material::is_textured_volume() const {
    return properties_.is(Property::Textured_volume);
}

inline bool Material::is_heterogeneous_volume() const {
    return properties_.is(Property::Heterogeneous_volume);
}

inline float Material::ior() const {
    return ior_;
}

inline Material::Radiance_sample::Radiance_sample(float2 uv, float pdf)
    : uvw{uv[0], uv[1], 0.f, pdf} {}

inline Material::Radiance_sample::Radiance_sample(float3 const& uvw, float pdf)
    : uvw{uvw[0], uvw[1], uvw[2], pdf} {}

inline float Material::Radiance_sample::pdf() const {
    return uvw[3];
}

}  // namespace scene::material

#endif
