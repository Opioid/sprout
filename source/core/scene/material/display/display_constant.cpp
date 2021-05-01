#include "display_constant.hpp"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "display_sample.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::display {

Constant::Constant(Sampler_settings sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Constant::sample(float3_p wo, Ray const& /*ray*/, Renderstate const& rs,
                                         Filter /*filter*/, Sampler& /*sampler*/,
                                         Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.set_common(rs, wo, emission_, emission_, alpha_);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    sample.set(fresnel::schlick_f0(ior_, rs.ior));

    return sample;
}

float3 Constant::evaluate_radiance(float3_p /*wi*/, float3_p /*uvw*/, float /*extent*/,
                                   Filter /*filter*/, Worker const& /*worker*/) const {
    return emission_;
}

float3 Constant::average_radiance(float /*area*/) const {
    return emission_;
}

void Constant::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

size_t Constant::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::display
