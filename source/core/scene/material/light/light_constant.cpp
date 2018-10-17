#include "light_constant.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "light_material_sample.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::light {

Constant::Constant(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : Material(sampler_settings, two_sided) {}

material::Sample const& Constant::sample(float3 const& wo, Renderstate const& rs,
                                         Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
                                         Worker const& worker, uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(emittance_.radiance(rs.area));

    return sample;
}

float3 Constant::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float area,
                                   uint64_t /*time*/, Sampler_filter /*filter*/,
                                   Worker const& /*worker*/) const noexcept {
    return emittance_.radiance(area);
}

float3 Constant::average_radiance(float area) const noexcept {
    float3 radiance = emittance_.radiance(area);

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
