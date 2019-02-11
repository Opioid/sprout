#include "light_material_sample.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::light {

const material::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate_f(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample::evaluate_b(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

float3 Sample::radiance() const noexcept {
    return radiance_;
}

void Sample::sample(Sampler& /*sampler*/, bxdf::Sample& result) const noexcept {
    result.reflection = float3(0.f);
    result.pdf        = 0.f;
}

bool Sample::is_pure_emissive() const noexcept {
    return true;
}

void Sample::set(float3 const& radiance) noexcept {
    radiance_ = radiance;
}

}  // namespace scene::material::light
