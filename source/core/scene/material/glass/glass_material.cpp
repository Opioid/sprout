#include "glass_material.hpp"
#include "base/math/vector4.inl"
#include "glass_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass::Glass(Sampler_settings sampler_settings) : Material(sampler_settings, false) {
    properties_.set(Property::Caustic);
}

material::Sample const& Glass::sample(float3_p wo, Ray const& /*ray*/, Renderstate const& rs,
                                      Filter filter, Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    if (normal_map_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key(), filter);

        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set_common(rs, wo, refraction_color_, float3(0.f), rs.alpha);
    sample.set(ior_, rs.ior);

    return sample;
}

void Glass::set_normal_map(Turbotexture const& normal_map) {
    normal_map_ = normal_map;
}

void Glass::set_refraction_color(float3_p color) {
    refraction_color_ = color;
}

size_t Glass::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::glass
