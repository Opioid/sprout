#pragma once

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
void Material_base::set_sample(f_float3 wo, Renderstate const& rs,
                               Texture_sampler_2D const& sampler, Sample& sample) const {
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
        surface    = surface_map_.sample_2(sampler, rs.uv);
        surface[0] = ggx::map_roughness(surface[0]);
    } else {
        surface[0] = roughness_;
        surface[1] = metallic_;
    }

    float3 radiance;
    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * emission_map_.sample_3(sampler, rs.uv);
    } else {
        radiance = float3::identity();
    }

    sample.layer_.set(color, radiance, ior_, constant_f0_, surface[0], surface[1]);
    sample.avoid_caustics_ = rs.avoid_caustics;
}

}  // namespace scene::material::substitute
