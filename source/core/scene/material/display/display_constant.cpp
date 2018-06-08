#include "display_constant.hpp"
#include "base/math/distribution/distribution_2d.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::display {

Constant::Constant(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

const material::Sample& Constant::sample(f_float3 wo, Renderstate const& rs,
                                         Sampler_filter /*filter*/, sampler::Sampler& /*sampler*/,
                                         Worker const& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.layer_.set(emission_, f0_, roughness_);

    return sample;
}

float3 Constant::sample_radiance(f_float3 /*wi*/, float2 /*uv*/, float /*area*/, float /*time*/,
                                 Sampler_filter /*filter*/, Worker const& /*worker*/) const {
    return emission_;
}

float3 Constant::average_radiance(float /*area*/) const {
    return emission_;
}

float Constant::ior() const {
    return ior_;
}

size_t Constant::num_bytes() const {
    return sizeof(*this);
}

void Constant::set_emission(float3 const& radiance) {
    emission_ = radiance;
}

void Constant::set_roughness(float roughness) {
    roughness_ = roughness;
}

void Constant::set_ior(float ior) {
    ior_ = ior;
    f0_  = fresnel::schlick_f0(1.f, ior);
}

size_t Constant::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::display
