#include "null_sample.hpp"
#include "base/math/vector3.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::null {

Sample::Sample() {
    properties_.set(Property::Can_evaluate, false);
}

bxdf::Result Sample::evaluate_f(float3_p /*wi*/) const {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample::evaluate_b(float3_p /*wi*/) const {
    return {float3(0.f), 0.f};
}

void Sample::sample(Sampler& /*sampler*/, rnd::Generator& /*rng*/, bxdf::Sample& result) const {
    result.reflection = float3(factor_);
    result.wi         = -wo_;
    result.pdf        = 1.f;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Straight_transmission);
}

}  // namespace scene::material::null
