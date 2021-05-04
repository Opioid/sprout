#include "light_constant.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Constant::Constant(Sampler_settings sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided) {
    properties_.set(Property::Pure_emissive);
}

material::Sample const& Constant::sample(float3_p wo, Ray const& /*ray*/, Renderstate const& rs,
                                         Filter /*filter*/, Sampler& /*sampler*/,
                                         Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    float3 const radiance = emittance_.radiance(worker.scene().light_area(rs.prop, rs.part));

    sample.set_common(rs, wo, radiance, radiance, 0.f);

    return sample;
}

float3 Constant::evaluate_radiance(float3_p /*wi*/, float3_p /*uvw*/, float extent,
                                   Filter /*filter*/, Worker& /*worker*/) const {
    return emittance_.radiance(extent);
}

float3 Constant::average_radiance(float area) const {
    return emittance_.radiance(area);
}

::light::Emittance& Constant::emittance() {
    return emittance_;
}

size_t Constant::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::light
