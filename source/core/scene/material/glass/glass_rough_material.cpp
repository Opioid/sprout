#include "glass_rough_material.hpp"
#include "base/math/vector4.inl"
#include "glass_rough_sample.hpp"

#include "scene/material/collision_coefficients.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_rough::Glass_rough(Sampler_settings sampler_settings) : Material(sampler_settings, false) {}

void Glass_rough::commit(Threads& /*threads*/, Scene const& /*scene*/) {
    properties_.set(Property::Caustic, alpha_ <= ggx::Min_alpha);
}

material::Sample const& Glass_rough::sample(float3_p wo, Ray const& /*ray*/, Renderstate const& rs,
                                            Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_rough>();

    auto& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float alpha;
    if (roughness_map_.is_valid()) {
        float const r = ggx::map_roughness(sampler.sample_1(roughness_map_, rs.uv, worker.scene()));

        alpha = r * r;
    } else {
        alpha = alpha_;
    }

    sample.set_common(rs, wo, refraction_color_, float3(0.f), alpha);
    sample.set(ior_, rs.ior);

    return sample;
}

void Glass_rough::set_normal_map(Texture const& normal_map) {
    normal_map_ = normal_map;
}

void Glass_rough::set_roughness_map(Texture const& roughness_map) {
    roughness_map_ = roughness_map;
}

void Glass_rough::set_refraction_color(float3_p color) {
    refraction_color_ = color;
}

void Glass_rough::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

size_t Glass_rough::sample_size() {
    return sizeof(Sample_rough);
}

}  // namespace scene::material::glass
