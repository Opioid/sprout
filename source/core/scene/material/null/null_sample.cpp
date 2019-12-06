#include "null_sample.hpp"
#include "base/math/vector3.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::null {

float3 const& Sample::base_shading_normal() const noexcept {
    return geo_n_;
}

bxdf::Result Sample::evaluate_f(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample::evaluate_b(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

void Sample::sample(Sampler& /*sampler*/, bxdf::Sample& result) const noexcept {
    result.reflection = float3(1.f);
    result.wi         = -wo_;
    result.pdf        = 1.f;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Straight_transmission);
}

bool Sample::ior_greater_one() const noexcept {
    return false;
}

bool Sample::evaluates_back(bool previously, bool /*same_side*/) const noexcept {
    return previously;
}

}  // namespace scene::material::null
