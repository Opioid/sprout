#include "light_constant.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Constant::Constant(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : Material(sampler_settings, two_sided) {}

material::Sample const& Constant::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         sampler::Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<Sample>();

    sample.set_basis(rs.geo_n, wo);

    sample.set(emittance_.radiance(worker.scene().light_area(rs.prop, rs.part)));

    return sample;
}

float3 Constant::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float area,
                                   Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return emittance_.radiance(area);
}

float3 Constant::average_radiance(float area, Scene const& /*scene*/) const noexcept {
    float3 const radiance = emittance_.radiance(area);

    if (is_two_sided()) {
        return 2.f * radiance;
    }

    return radiance;
}

float Constant::ior() const noexcept {
    return 1.5f;
}

bool Constant::has_emission_map() const noexcept {
    return false;
}

size_t Constant::num_bytes() const noexcept {
    return sizeof(*this);
}

::light::Emittance& Constant::emittance() noexcept {
    return emittance_;
}

size_t Constant::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::light
