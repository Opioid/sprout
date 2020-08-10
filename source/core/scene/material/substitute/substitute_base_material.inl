#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL

#include "base/math/vector3.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

static inline float3 f0_to_a(float3 const& f0) {
    return 5.f * sqrt(max(0.95f - f0, 0.0001f));
}

static inline float3 f0_to_a_b(float3 const& f0) {
    return 6.f * (1.f - f0);
}

template <typename Sample>
void Material_base::set_sample(float3 const& wo, Renderstate const& rs, float ior_outside,
                               Texture_sampler_2D const& sampler, Worker const& worker,
                               Sample& sample) const {
    sample.set_basis(rs.geo_n, rs.n, wo);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float3 color;
    if (color_map_.is_valid()) {
        color = color_map_.sample_3( sampler, rs.uv);
    } else {
        color = color_;
    }

    float2 surface;
    if (surface_map_.is_valid()) {
        surface = surface_map_.sample_2( sampler, rs.uv);

        float const r = ggx::map_roughness(surface[0]);

        surface[0] = r * r;
    } else {
        surface[0] = alpha_;
        surface[1] = metallic_;
    }

    float3 radiance;
    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * emission_map_.sample_3( sampler, rs.uv);
    } else {
        radiance = float3(0.f);
    }

    sample.set_radiance(radiance);
    sample.base_.set(color, fresnel::schlick_f0(ior_, ior_outside), surface[0], surface[1],
                     rs.avoid_caustics);
}

}  // namespace scene::material::substitute

#endif
