#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL

#include "base/math/vector3.inl"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

template <typename Sample>
void Material_base::set_sample(float3 const& wo, Renderstate const& rs, float ior_outside,
                               Texture_sampler_2D const& sampler, Sample& sample) const noexcept {
    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float3 color;
    if (color_map_.is_valid()) {
        color = color_map_.sample_3(sampler, rs.uv);
    } else {
        color = color_;
    }

    float2 surface;
    if (surface_map_.is_valid()) {
        surface = surface_map_.sample_2(sampler, rs.uv);

        float const r = ggx::map_roughness(surface[0]);

        surface[0] = r * r;
    } else {
        surface[0] = alpha_;
        surface[1] = metallic_;
    }

    float3 radiance;
    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * emission_map_.sample_3(sampler, rs.uv);
    } else {
        radiance = float3(0.f);
    }

    sample.base_.set(color, radiance, fresnel::schlick_f0(ior_, ior_outside), surface[0],
                     surface[1]);

    sample.avoid_caustics_ = rs.avoid_caustics;
}

}  // namespace scene::material::substitute

#endif
