#include "display_constant.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::display {

Constant::Constant(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Constant::sample(float3 const&      wo, Ray const& /*ray*/,
                                         Renderstate const& rs, Filter /*filter*/,
                                         sampler::Sampler& /*sampler*/, Worker const& worker) const
    noexcept {
    auto& sample = worker.sample<Sample>(rs.sample_level);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(emission_, f0_, alpha_);

    return sample;
}

float3 Constant::evaluate_radiance(float3 const& /*wi*/, float2 /*uv*/, float /*area*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const noexcept {
    return emission_;
}

float3 Constant::average_radiance(float /*area*/) const noexcept {
    return emission_;
}

float Constant::ior() const noexcept {
    return ior_;
}

size_t Constant::num_bytes() const noexcept {
    return sizeof(*this);
}

void Constant::set_emission(float3 const& radiance) noexcept {
    emission_ = radiance;
}

void Constant::set_roughness(float roughness) noexcept {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

void Constant::set_ior(float ior) noexcept {
    ior_ = ior;
    f0_  = fresnel::schlick_f0(1.f, ior);
}

size_t Constant::sample_size() noexcept {
    return sizeof(Sample);
}

}  // namespace scene::material::display
