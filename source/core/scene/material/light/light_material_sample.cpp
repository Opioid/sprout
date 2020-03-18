#include "light_material_sample.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::light {

Sample::Sample() {
    properties_.set(Property::Pure_emissive);
}

float3 const& Sample::base_shading_normal() const {
    return geo_n_;
}

bxdf::Result Sample::evaluate_f(float3 const& /*wi*/) const {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample::evaluate_b(float3 const& /*wi*/) const {
    return {float3(0.f), 0.f};
}

float3 Sample::radiance() const {
    return radiance_;
}

void Sample::sample(Sampler& /*sampler*/, bxdf::Sample& result) const {
    result.reflection = float3(0.f);
    result.pdf        = 0.f;
}

void Sample::set(float3 const& radiance) {
    radiance_ = radiance;
}

}  // namespace scene::material::light
