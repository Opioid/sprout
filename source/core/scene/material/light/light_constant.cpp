#include "light_constant.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Constant::Constant(Sampler_settings const& sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided) {}

material::Sample const& Constant::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.set_basis(rs.geo_n, rs.n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(emittance_.radiance(worker.scene().light_area(rs.prop, rs.part)));

    return sample;
}

float3 Constant::evaluate_radiance(float3 const& /*wi*/, float3 const& /*uvw*/, float extent,
                                   Filter /*filter*/, Worker const& /*worker*/) const {
    return emittance_.radiance(extent);
}

float3 Constant::average_radiance(float area) const {
    float3 const radiance = emittance_.radiance(area);

    if (is_two_sided()) {
        return 2.f * radiance;
    }

    return radiance;
}

::light::Emittance& Constant::emittance() {
    return emittance_;
}

size_t Constant::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::light
