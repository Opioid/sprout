#include "null_sample.hpp"
#include "base/math/vector3.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::null {

const material::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

void Sample::sample(Sampler& /*sampler*/, bxdf::Sample& result) const noexcept {
    result.reflection = float3(1.f);
    result.wi         = -wo_;
    result.pdf        = 1.f;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Specular_transmission);
}

bool Sample::ior_greater_one() const noexcept {
    return false;
}

bool Sample::do_evaluate_back(bool previously, bool /*same_side*/) const noexcept {
    return previously;
}

}  // namespace scene::material::null
