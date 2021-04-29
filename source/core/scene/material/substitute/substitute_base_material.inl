#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_INL

#include "base/math/vector3.inl"

#include "image/texture/texture.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.hpp"
#include "substitute_sample.hpp"

namespace scene::material::substitute {

static inline float3 f0_to_a(float3_p f0) {
    return 5.f * sqrt(max(0.95f - f0, 0.0001f));
}

static inline float3 f0_to_a_b(float3_p f0) {
    return 6.f * (1.f - f0);
}

template <typename Sample>
void Material_base::set_sample(float3_p wo, Renderstate const& rs, float ior_outside,
                               Texture_sampler_2D const& sampler, Worker const& worker,
                               Sample& sample) const {
    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float3 color;
    if (color_map_.is_valid()) {
        color = sampler.sample_3(color_map_, rs.uv, worker.scene());
    } else {
        color = color_;
    }

    float alpha;
    float metallic;
    if (surface_map_.is_valid()) {
        float2 const surface = sampler.sample_2(surface_map_, rs.uv, worker.scene());

        float const r = ggx::map_roughness(surface[0]);

        alpha    = r * r;
        metallic = surface[1];
    } else {
        alpha    = alpha_;
        metallic = metallic_;
    }

    float3 radiance;
    if (emission_map_.is_valid()) {
        radiance = emission_factor_ * sampler.sample_3(emission_map_, rs.uv, worker.scene());
    } else {
        radiance = float3(0.f);
    }

    //    if (alpha > ggx::Min_alpha) {
    //        alpha = std::max(rs.alpha, alpha);
    //    }

    // std::max(surface[0], std::min(float(depth * depth) * 0.025f, 1.f))

    sample.set_common(rs, wo, color, radiance, alpha);

    sample.base_.set(color, fresnel::schlick_f0(ior_, ior_outside), metallic);
}

}  // namespace scene::material::substitute

#endif
